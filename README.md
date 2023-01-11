# Serial Communication Library

This repository contains a multiplatform serial communication library written in C language.

## Cloning

This repository uses [submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules). When cloning this repo, ensure the option `--recursive` is present.

## Supported platforms

Currently, the library is supported for the following platforms (hosts):

* linux-x64
* linux-x86
* windows-x64
* windows-x86

## Compiling

In order to compile this library, the following tools must be present on the system (the validated build machine was running Ubuntu 20.04):

* [GNU Make](https://www.gnu.org/software/make/) (v.4.2+)

  * On ubuntu, GNU make can be installed via apt:

    ```sh
    sudo apt install build-essential
    ```

* [GCC](https://gcc.gnu.org/) (v.9.4+)

  * On ubuntu, GCC can be installed via apt:

    ```sh
    sudo apt install build-essential gcc-multilib
    ```

* [Mingw-w64](https://www.mingw-w64.org/) GCC (v.9.3+)

  * On ubuntu, Mingw-w64 can be installed via apt:

    ```sh
    sudo apt install gcc-mingw-w64 gcc-mingw-w64-i686
    ```

### Building

Just call from the project root directory:

```sh
make HOST=<host> O=output/<host>
```

The result library will be located in directory **`output/<host>/dist/lib`** (distribution headers are located into directory **`output/<host>/dist/include`**)

## Licensing

This project is distributed under MIT License. Please see the [LICENSE](LICENSE) file for details on copying and distribution.

## Contact

For general information visit the main project site at https://github.com/ljbo82/serial.
