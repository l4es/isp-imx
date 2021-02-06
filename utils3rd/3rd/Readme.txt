=======================================================================================================================
Note: Must build boost and cpp-netlib before build code in appshell folder.
=======================================================================================================================

Here are the steps to compile Boost and CPP-netlib for reference.
=======================================================================================================================
Follow steps are building 'debug' mode of ARM64.
Build release mode, just replace the 'debug' keyword on the command line.

$ export MY_DEV=~/workspace/utils3rd/3rd

1. Build boost
------------------------------------------------------------------------------------------------------------------------
    (1) Open a new terminal to execute follow command, or b2 file will be wrong format to execute.Use gcc to build bjam.
        $ cd $MY_DEV/boost
        $ ./bootstrap.sh --with-libraries=date_time,filesystem,regex,system,thread

    (2) Edit Modify the configuration file "project-config.jam" to use the Poky tool chain by replacing the line with “using gcc” by:

        # Compiler configuration. This definition will be used unless
        # you already have defined some toolsets in your user-config.jam
        # file.
        if ! gcc in [ feature.values <toolset> ]
        {
        --    using gcc ;
        ++    using gcc : poky : /opt/fsl-imx-xwayland/5.4-zeus/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc ;
        }

    (3) build
        $ mkdir -p ../build/build-boost-aarch64-poky-linux-debug
        $ ./b2 debug toolset=gcc-poky --build-dir=../build/build-boost-aarch64-poky-linux-debug --stagedir=../build/build-boost-aarch64-poky-linux-debug cxxflags="-I/opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include/c++/9.2.0 -I/opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include/c++/9.2.0/aarch64-poky-linux -I/opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include"

    (4) copy library file to prebuild path
        $ cp ../build/build-boost-aarch64-poky-linux-debug/lib/* build/ARM64/debug/


2. build cpp-nelib
------------------------------------------------------------------------------------------------------------------------
    (1) add toolchain path to environment
        $ source /opt/fsl-imx-xwayland/5.4-zeus/environment-setup-aarch64-poky-linux

    (2) build
        $ cd $MY_DEV/cpp-netlib
        $ mkdir ../build/build-cpp-netlib-aarch64-poky-linux-debug && cd ../build/build-cpp-netlib-aarch64-poky-linux-debug
        $ cmake -DCMAKE_BUILD_TYPE=debug -Wno-dev -DBOOST_ROOT="$MY_DEV/boost" -DBOOST_LIBRARYDIR="$MY_DEV/build/build-boost-aarch64-poky-linux-debug/lib" -DCPP-NETLIB_BUILD_TESTS=OFF -DCPP-NETLIB_BUILD_EXAMPLES=OFF -DCPP-NETLIB_ENABLE_HTTPS=OFF ../../cpp-netlib
        $ make -j4

    (3) copy library file to prebuild path
        $ cp libs/network/src/*.a  ../../cpp-netlib/prebuilt/ARM64/debug/


=======================================================================================================================
Follow steps are building 'debug' mode of X86.
Build 'release' mode, just replace the 'debug' keyword on the command line.

$ export MY_DEV=~/workspace/utils3rd/3rd

1. Build boost
------------------------------------------------------------------------------------------------------------------------
    (1) Open a new terminal to execute follow command, or b2 file will be wrong format to execute.Use gcc to build bjam.
        $ cd $MY_DEV/boost
        $ ./bootstrap.sh --with-libraries=date_time,filesystem,regex,system,thread

    (2) build
        $ mkdir -p ../build/build-boost-ubuntu_16_04_x86_64-debug
        $ ./b2 debug --build-dir=../build/build-boost-ubuntu_16_04_x86_64-debug --stagedir=../build/build-boost-ubuntu_16_04_x86_64-debug

    (3) copy library file to prebuild path
        $ cp ../build/build-boost-ubuntu_16_04_x86_64-debug/lib/* build/X86/debug/


2. build cpp-nelib
------------------------------------------------------------------------------------------------------------------------
    (1) build
        $ cd $MY_DEV/cpp-netlib
        $ mkdir -p build/build-cpp-netlib-ubuntu_16_04_x86_64-debug && cd build/build-cpp-netlib-ubuntu_16_04_x86_64-debug
        $ cmake -DCMAKE_BUILD_TYPE=debug -Wno-dev -DBOOST_ROOT="$MY_DEV/boost" -DBOOST_LIBRARYDIR="$MY_DEV/build/build-boost-ubuntu_16_04_x86_64-debug/lib" -DCPP-NETLIB_BUILD_TESTS=OFF -DCPP-NETLIB_BUILD_EXAMPLES=OFF -DCPP-NETLIB_ENABLE_HTTPS=OFF ../../../cpp-netlib
        $ make -j4

    (2) copy library file to prebuild path
        $ cp libs/network/src/*.a  ../../../cpp-netlib/prebuilt/X86/debug/


