package main

import (
	pkg "github.com/jurgen-kluft/xenv/package"
)

func main() {
	xcode.Init()
	xcode.Generate(pkg.GetPackage())
}
