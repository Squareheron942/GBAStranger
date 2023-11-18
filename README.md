[![cpp-linter](https://github.com/Meepster99/GBAStranger/actions/workflows/cpp-linter.yml/badge.svg?kill_cache=1)](https://github.com/Meepster99/GBAStranger/actions/workflows/cpp-linter.yml)
[![Pylint](https://github.com/Meepster99/GBAStranger/actions/workflows/pylint.yml/badge.svg?kill_cache=1)](https://github.com/Meepster99/GBAStranger/actions/workflows/pylint.yml)

## Instructions to build C++ code.

install [devkitpro](https://devkitpro.org/) and [python](https://www.python.org/downloads/) >= 3.11

clone the repo, and do `git submodule update --init --recursive`

run convertAllAssets.py, located in the misc folder

make the makefile in code/ preferably with -j8 or -j16 and LOG=1 or PROFILE=1


## Instructions to build rust code.

Use `cargo build -vv` to compile the rust code. This will take a while the first time since it converts all the
room JSONs to rust constants at compile time. These will be cached after the first time. To reduce output for later
builds, just run `cargo build`. 

To see documentation of all the constants and structs generated by the build file (and defined otherwise in your source 
code), use `cargo doc --open --document-private-items`. 

If you have mGBA installed, you can build and run code directly using `cargo run`. To run with optimizations,
use `cargo run --release`. 

## License

You can do whatever you want, provided you provide credit to Meepster99
