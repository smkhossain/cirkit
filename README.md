# CirKit

CirKit is a software library and framework for logic synthesis.

## Requirements

The following software is required in order to build CirKit

* git
* cmake (at least version 3.0.0)
* g++ (at least version 4.9.0) or clang++ (at least version 3.5.0)
* boost (at least version 1.56.0)
* GNU MP, and its C++ interface GMP++
* GNU readline

In *Ubuntu* the packages can be installed with

    sudo apt-get install build-essential git g++ cmake libboost-all-dev libgmp3-dev libxml2-dev

In *arch* the packages can be installed with

    sudo pacman -S base-devel git g++ cmake boost boost-libs gmp libxml2

## Build and Run CirKit

After extracting or cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake ..
    make external
    make cirkit

CirKit can be executed with

    build/programs/cirkit

Check the [documentation](http://msoeken.github.io/cirkit_doc.html) for more details.

## Build and Run RevKit

After extracting or cloning CirKit perform the following steps

    mkdir build
    cd build
    cmake -Denable_cirkit-addon-reversible=ON -Denable_cirkit-addon-formal=ON ..
    make external
    make revkit

RevKit can be executed with

    build/programs/revkit
    
Check the [documentation](http://msoeken.github.io/cirkit_doc.html) for more details.

## Build CirKit Addons

CirKit can be extended by addons, you can learn more in the `addons/README.md`
file.  The addons are not included into the build process by default.  The
easiest way to enable addons is by typing `ccmake ..` in the build directory
which opens the ncurses GUI of cmake.  An addon can be enabled by toggling the
flag at the entry `enable_cirkit-addon-*`.  Afterwards, press `c` followed by
`g` and then recompile with `make`.

## Package Manager

CirKit uses some external (mainly academic) programs that are typically not
shipped with Linux distributions.  To ease their installation CirKit provides
its own small package manager that can be invoked via `utils/tools.py`.  Run

    utils/tools.py commands

to learn how it can be executed.  The programs are automatically downloaded and
build, binaries are installed in `ext/bin`.
