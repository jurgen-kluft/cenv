#ifndef __CENV_ENV_PUBLIC_H__
#define __CENV_ENV_PUBLIC_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_debug.h"

namespace ncore
{
    namespace xenv
    {
        struct env_t;
        typedef env_t* penv_t;                

        // init the env variable
        //
        // @return              the env variable
        //
        penv_t env_init(void);

        // exit the env variable
        //
        // @param env   the env variable
        //
        void env_exit(penv_t env);

        // the env variable count
        //
        // @param env   the env variable
        //
        // @return              the env variable count
        //
        uint_t env_size(penv_t env);

        // load the env variable from the given name
        //
        // @code
        //
        //    // init env
        //    penv_t env = env_init();
        //    if (env)
        //    {
        //        // load variable
        //        if (env_load(env, "PATH"))
        //        {
        //            // dump values
        //            for_all_if (char const*, value, env, value)
        //            {
        //                // trace
        //                trace_i("    %s", value);
        //            }
        //        }
        //
        //        // exit env
        //        env_exit(env);
        //    }
        // @endcode
        //
        // @param env   the env variable
        // @param name          the variable name
        //
        // @return              the count of the variable value
        //
        uint_t env_load(penv_t env, char const* name);

        // save the env variable and will overwrite it
        //
        // we will remove this env variable if env is null or empty
        //
        // @code
        //
        //    // init env
        //    penv_t env = env_init();
        //    if (env)
        //    {
        //        // insert values
        //        env_insert(env, "/xxx/0", false);
        //        env_insert(env, "/xxx/1", false);
        //        env_insert(env, "/xxx/2", false);
        //        env_insert(env, "/xxx/3", false);
        //        // save variable
        //        env_save(env, "PATH");
        //        // exit env
        //        env_exit(env);
        //    }
        // @endcode
        //
        // @param env   the env variable
        // @param name          the variable name
        //
        // @return              true or false
        //
        bool env_save(penv_t env, char const* name);

        // get the env variable from the given index
        //
        // @code
        //
        //            // init env
        //            penv_t env = env_init();
        //            if (env)
        //            {
        //                // load variable
        //                if (env_load(env, "PATH"))
        //                {
        //                    char const* value = env_at(env, 0);
        //                    if (value)
        //                    {
        //                        // ...
        //                    }
        //                }
        //
        //                // exit env
        //                env_exit(env);
        //            }
        // @endcode
        //
        //
        // @param env   the env variable
        // @param index         the variable index
        //
        // @return              the variable value
        //
        char const* env_at(penv_t env, uint_t index);

        // replace the env variable and will overwrite it
        //
        // we will clear env and overwrite it
        //
        // @param env   the env variable
        // @param value         the variable value, will clear it if be null
        //
        // @return              true or false
        //
        bool env_replace(penv_t env, char const* value);

        // set the env variable
        //
        // @param env   the env variable
        // @param value         the variable value
        // @param to_head       insert value into the head?
        //
        // @return              true or false
        //
        bool env_insert(penv_t env, char const* value, bool to_head);

#ifdef TARGET_DEBUG
        // dump the env variable
        //
        // @param env   the env variable
        // @param name          the variable name
        //
        void env_dump(penv_t env, char const* name);
#endif

        // get the first env variable value
        //
        // @code
        //
        //            char value[TB_PATH_MAXN];
        //            if (env_first("HOME", value, sizeof(value)))
        //            {
        //                // ...
        //            }
        //
        // @endcode
        //
        // @param name          the variable name
        // @param value         the variable value
        // @param maxn          the variable value maxn
        //
        // @return              the variable value size
        //
        uint_t env_first(char const* name, char* value, uint_t maxn);

        // get the env variable values
        //
        // @code
        //
        //            char value[TB_PATH_MAXN];
        //            if (env_get("HOME", value, sizeof(value)))
        //            {
        //                // ...
        //            }
        //
        // @endcode
        //
        // @param name          the variable name
        // @param values        the variable values, separator: windows(';') or other(';')
        // @param maxn          the variable values maxn
        //
        // @return              the variable values size
        //
        uint_t env_get(char const* name, char* values, uint_t maxn);

        // set the env variable values
        //
        // we will set all values and overwrite it
        //
        // @param name          the variable name
        // @param values        the variable values, separator: windows(';') or other(';')
        //
        // @return              true or false
        //
        bool env_set(char const* name, char const* values);

        // add the env variable values and not overwrite it
        //
        // @param name          the variable name
        // @param values        the variable values, separator: windows(';') or other(';')
        // @param to_head       add value into the head?
        //
        // @return              true or false
        //
        bool env_add(char const* name, char const* values, bool to_head);

        // remove the given env variable
        //
        // @param name          the variable name
        //
        // @return              true or false
        //
        bool env_remove(char const* name);

    } // namespace xenv

} // namespace ncore

#endif //< __CENV_ENV_PUBLIC_H__