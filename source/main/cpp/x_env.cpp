#include "xbase/x_target.h"

#if defined TARGET_PC

#    include "ole32.h"
#    include "user32.h"
#    include "ws2_32.h"
#    include "mswsock.h"
#    include "shell32.h"
#    include "dbghelp.h"
#    include "kernel32.h"
#    include "iphlpapi.h"

#elif defined TARGET_MAC

#    include <stdlib.h>
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <stdio.h>
#    include <dirent.h>
#    include <unistd.h>
#    include <errno.h>

#endif

#include "xbase/x_context.h"
#include "xenv/x_env.h"
#include "xenv/private/x_assert.h"

namespace ncore
{
    namespace nenv
    {
// the separator
#if defined(TARGET_OS_WINDOWS)
#    define TM_ENVIRONMENT_SEP ';'
#else
#    define TM_ENVIRONMENT_SEP ':'
#endif

#if defined(TARGET_OS_WINDOWS)

        static char* env_get_impl(char const* name, uint_t* psize)
        {
            // check
            assert_and_check_return_val(name, 0);

            // done
            bool     ok      = false;
            uint_t   size    = 0;
            uint_t   maxn    = 256;
            char*    value   = null;
            wchar_t* value_w = null;
            do
            {
                // make value_w
                value_w = (wchar_t*)malloc0(sizeof(wchar_t) * maxn);
                assert_and_check_break(value_w);

                // make name
                wchar_t name_w[512];
                uint_t  name_n = atow(name_w, name, arrayn(name_w));
                assert_and_check_break(name_n != -1);

                // get it
                size = (uint_t)kernel32()->GetEnvironmentVariableW(name_w, value_w, (DWORD)maxn);
                if (!size)
                {
                    // error?
                    if (ERROR_ENVVAR_NOT_FOUND == GetLastError())
                    {
                        // trace
                        // trace_d("env variable(%s) does not exist", name);
                    }

                    break;
                }
                else if (size > maxn)
                {
                    // grow space
                    value_w = (wchar_t*)ralloc(value_w, sizeof(wchar_t) * (size + 1));
                    assert_and_check_break(value_w);

                    // get it
                    size = (uint_t)kernel32()->GetEnvironmentVariableW(name_w, value_w, (DWORD)size + 1);
                    assert_and_check_break(size);
                }

                // make value
                maxn  = (size << 2) + 1;
                value = (char*)malloc0(sizeof(char) * (maxn + 1));
                assert_and_check_break(value);

                // save value
                if ((size = wtoa(value, value_w, maxn)) == -1)
                    break;

                // save size
                if (psize)
                    *psize = size;

                // ok
                ok = true;

            } while (0);

            // failed?
            if (!ok)
            {
                // exit value
                if (value)
                    free(value);
                value = null;
            }

            // exit value_w
            if (value_w)
                free(value_w);
            value_w = null;

            // ok?
            return value;
        }

        static bool env_set_impl(char const* name, char const* value)
        {
            // check
            assert_and_check_return_val(name, false);

            // done
            bool     ok      = false;
            wchar_t* value_w = null;
            uint_t   value_n = 0;
            do
            {
                // make name
                wchar_t name_w[512];
                uint_t  name_n = atow(name_w, name, arrayn(name_w));
                assert_and_check_break(name_n != -1);

                // exists value?
                if (value)
                {
                    // make value
                    value_n = strlen(value);
                    value_w = (wchar_t*)malloc0(sizeof(wchar_t) * (value_n + 1));
                    assert_and_check_break(value_w);

                    // init value
                    if (atow(value_w, value, value_n + 1) == -1)
                        break;

                    // set it
                    if (!kernel32()->SetEnvironmentVariableW(name_w, value_w))
                        break;
                }
                // remove this variable
                else
                {
                    // remove it
                    if (!kernel32()->SetEnvironmentVariableW(name_w, null))
                        break;
                }

                // ok
                ok = true;

            } while (0);

            // exit data
            if (value_w)
                free(value_w);
            value_w = null;

            // ok?
            return ok;
        }

        uint_t env_load(env_t* env, char const* name)
        {
            // check
            assert_and_check_return_val(env && name, 0);

            // clear env first
            vector_clear(env);

            // get values
            char const* values = env_get_impl(name, null);
            check_return_val(values, 0);

            // init value string
            string_t value;
            if (string_init(&value))
            {
                // done
                char const* p = values;
                char        c = '\0';
                while (1)
                {
                    // the character
                    c = *p++;

                    // make value
                    if (c != ';' && c)
                        string_chrcat(&value, c);
                    else
                    {
                        // save value to env
                        if (string_size(&value))
                            vector_insert_tail(env, string_cstr(&value));

                        // clear value
                        string_clear(&value);

                        // end?
                        check_break(c);
                    }
                }

                // exit value string
                string_exit(&value);
            }

            // exit values
            if (values)
                free(values);
            values = null;

            // ok?
            return vector_size(env);
        }

        bool env_save(env_t* env, char const* name)
        {
            // check
            assert_and_check_return_val(env && name, false);

            // empty? remove this env variable
            if (!vector_size(env))
                return env_set_impl(name, null);

            // init values string
            string_t values;
            if (!string_init(&values))
                return false;

            // make values string
            for_all_if(char const*, value, env, value)
            {
                // the single value cannot exist ';'
                assertf(!strchr(value, ';'), "invalid value: %s", value);

                // append value
                string_cstrcat(&values, value);
                string_chrcat(&values, ';');
            }

            // strip the last ';'
            string_strip(&values, string_size(&values) - 1);

            // save variable
            bool ok = env_set_impl(name, string_cstr(&values));

            // exit values string
            string_exit(&values);

            // ok?
            return ok;
        }

        uint_t env_first(char const* name, char* value, uint_t maxn)
        {
            // check
            assert_and_check_return_val(name && value && maxn, 0);

            // get it
            uint_t size = 0;
            char*  data = env_get_impl(name, &size);
            if (data && size < maxn)
            {
                // copy it
                strncpy(value, data, size);
                value[size] = '\0';

                // only get the first one if exists multiple values
                char* p = strchr(value, ';');
                if (p)
                {
                    // strip it
                    *p = '\0';

                    // update size
                    size = p - value;
                }
            }
            else
                size = 0;

            // exit data
            if (data)
                free(data);
            data = null;

            // ok?
            return size;
        }

        bool env_remove(char const* name)
        {
            // check
            assert_and_check_return_val(name, false);

            // remove it
            return env_set_impl(name, null);
        }

#elif defined(TARGET_MAC)

        uint_t env_load(env_t* env, char const* name)
        {
            // check
            assert_and_check_return_val(env && name, 0);

            // clear env first
            vector_clear(env);

            // get values
            char const* values = getenv(name);
            check_return_val(values, 0);

            // init value string
            string_t value;
            if (!string_init(&value))
                return 0;

            // done
            char const* p = values;
            char        c = '\0';
            while (1)
            {
                // the character
                c = *p++;

                // make value
                if (c != ':' && c)
                    string_chrcat(&value, c);
                else
                {
                    // save value to env
                    if (string_size(&value))
                        vector_insert_tail(env, string_cstr(&value));

                    // clear value
                    string_clear(&value);

                    // end?
                    check_break(c);
                }
            }

            // exit value string
            string_exit(&value);

            // ok?
            return vector_size(env);
        }

        bool env_save(env_t* env, char const* name)
        {
            // check
            assert_and_check_return_val(env && name, false);

            // empty? remove this env variable
            if (!vector_size(env))
                return !unsetenv(name);

            // init values string
            string_t values;
            if (!string_init(&values))
                return false;

            // make values string
            for_all_if(char const*, value, env, value)
            {
                // the single value cannot exist ':'
                assertf(!strchr(value, ':'), "invalid value: %s", value);

                // append value
                string_cstrcat(&values, value);
                string_chrcat(&values, ':');
            }

            // strip the last ':'
            string_strip(&values, string_size(&values) - 1);

            // save variable
            bool        ok;
            char const* value_cstr = string_cstr(&values);
            if (value_cstr)
                ok = !setenv(name, value_cstr, 1);
            else
                ok = !unsetenv(name);

            // exit values string
            string_exit(&values);

            // ok?
            return ok;
        }

        uint_t env_first(char const* name, char* value, uint_t maxn)
        {
            // check
            assert_and_check_return_val(name && value && maxn, 0);

            // get it
            char const* data = getenv(name);
            check_return_val(data, 0);

            // the value size
            uint_t size = strlen(data);
            check_return_val(size, 0);

            // the space is not enough
            assert_and_check_return_val(size < maxn, 0);

            // copy it
            strlcpy(value, data, maxn);
            value[size] = '\0';

            // only get the first one if exists multiple values
            char* p = strchr(value, ':');
            if (p)
            {
                // strip it
                *p = '\0';

                // update size
                size = p - value;
            }

            // ok
            return size;
        }

        bool env_remove(char const* name)
        {
            // check
            assert_and_check_return_val(name, false);

            // remove it
            return !unsetenv(name);
        }

#endif

    } // namespace nenv
} // namespace ncore
