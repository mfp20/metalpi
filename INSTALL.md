
In order to build the default distribution the quickstart in the README is enough. In the case you are willing to customize the code you'll need to build the vc4-toolchain as well as the arm-none-eabi toolchain. Then `make` is used to build the components.

## Build vc4 toolchain

The repo includes the [vc4-toolchain](https://github.com/itszor/vc4-toolchain) submodule. Be sure to pull the submodules then run 

`$ ./build-all.sh 2>&1 | tee build.log`

This will give you a vc4-targeted toolchain in $(pwd)/prefix/bin. Compilers for both C and C++ should have been built.


## Build  arm-none-eabi toolchain

You can build your own toolchain or use one of the many prebuilt toolchains available online ([example](http://newos.org/toolchains/arm-eabi-5.3.0-Linux-x86_64.tar.xz)).
