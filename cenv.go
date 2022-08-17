package main

import (
	ccode "github.com/jurgen-kluft/ccode"
	cpkg "github.com/jurgen-kluft/cenv/package"
)

func main() {
	ccode.Generate(cpkg.GetPackage())
}
