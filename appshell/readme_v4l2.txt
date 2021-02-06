Readme

1) Create Environment:
----------------------------------------------
   export ARCH="arm64"
   export CROSS_COMPILE="aarch64-linux-gnu-"
   source /opt/fsl-imx-xwayland/5.4-zeus/environment-setup-aarch64-poky-linux


2) create a build directory
----------------------------------------------
   mkdir build
   cd build


3) Top build (V4L2 compile)
  cd appshell
  mkdir build&&cd build

tuning less on FPGA
  cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=X86 -DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=0 -DSERVER_LESS=1  -DSUBDEV_V4L2=1 .. -Wno-dev

tuning less on EVK
  cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 -DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 -DSERVER_LESS=1  -DSUBDEV_V4L2=1 -DENABLE_IRQ=1 .. -Wno-dev

tuning tool on EVK
  cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 -DAPPMODE=NATIVE -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 -DSUBDEV_V4L2=1 -DSERVER_LESS=0  .. -Wno-dev

4) Sub module build build-flow (optional)
----------------------------------------------
   ISP product line:
   example: -DISP_VERSION=ISP8000NANO_V1802

   Support cross compile target platform: X86 (default), ARM64, ARMV7, CSKY
   example: -DISP_VERSION=ARM64

   Support app mode: NATIVE (default), V4L2, MEDIACONTROL
   example: -DAPPMODE=V4L2

   Full command:
   mkdir build&&cd build
   cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 -DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 ..
   make -j4


5) Test:
-------------------------------------------
   cd generate/debug/bin

