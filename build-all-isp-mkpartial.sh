#!/bin/sh

if [ "$1" != "" ]; then
    #echo "Arg 1 is BUILD_TYPE: Release or Debug ->"
    #echo $1
    BUILD_TYPE=$1
else
    BUILD_TYPE=Release
fi

if [ "$2" != "" ]; then
    #echo "Arg 2 is Branch: -> "
    #echo $2
    BRANCH=$2
else
    BRANCH=temp_partial_branch
fi

BUILD_TYPE=`echo ${BUILD_TYPE}| tr '[:upper:]' '[:lower:]'`
BUILD=partial

echo "BUILD_TYPE: ${BUILD_TYPE}"
echo "BRANCH: ${BRANCH}"

# Step 0 
rm -rf appshell/build
mkdir -p appshell/build
cd appshell/build

git reset HEAD --hard
git clean -fd

git branch ${BRANCH}
git checkout ${BRANCH}

# Step 1 
echo "Make Partial build --------------------->"
if [ "$BUILD_TYPE" = 'release' ]
then
echo "Release build --------------------->"

cmake -DCMAKE_BUILD_TYPE=release -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 -DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 -DSERVER_LESS=1 -DSUBDEV_V4L2=1 -DENABLE_IRQ=1 .. -Wno-dev


else
echo "Debug build   --------------------->"

cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 -DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 -DSERVER_LESS=1 -DSUBDEV_V4L2=1 -DENABLE_IRQ=1 .. -Wno-dev

fi
echo "Starting Build --------------------->"
# Step 2 
make -j4

echo "Pruning build --------------------->"

# Step 3 and 4
cd -
#cd dewarp/proprietories/
#sh mkpartitionbuild.sh
#cd -

# Step 5 and 6 
cd units/mkrel/ISP8000NANO_V1802
sh mkpartitionbuild.sh
cd -

rm -rf appshell/build
mkdir -p appshell/build
cd appshell/build

echo "Starting rebuild --------------------->"

# Step 7 
if [ "$BUILD_TYPE" = 'release' ]
then
echo "Release build --------------------->"

cmake -DCMAKE_BUILD_TYPE=release -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 \
-DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 \
-DSERVER_LESS=1 -DSUBDEV_V4L2=1 -DENABLE_IRQ=1 -DPARTITION_BUILD=0 -D3A_SRC_BUILD=0 .. -Wno-dev

else
echo "Debug build   --------------------->"

cmake -DCMAKE_BUILD_TYPE=debug -DISP_VERSION=ISP8000NANO_V1802 -DPLATFORM=ARM64 \
-DAPPMODE=V4L2 -DQTLESS=1 -DFULL_SRC_COMPILE=1 -DWITH_DWE=1 -DWITH_DRM=1 \
-DSERVER_LESS=1 -DSUBDEV_V4L2=1 -DENABLE_IRQ=1 -DPARTITION_BUILD=0 -D3A_SRC_BUILD=0 .. -Wno-dev

fi

#Step 8 
make -j4
cd -

echo "Rebuild completed  --------------------->"

git rm -rf  tuning-*
git status -s | grep "?? " |  gawk '{print $2}' |   xargs git add 
git status -s | grep " D " |  gawk '{print $2}' |   xargs git rm 
git commit -s -m "New partial build ${BRANCH}"

echo "Packaging the build --------------------->"

# copy the build results
BUILD_OUTPUT_DIR=build_output_${BUILD_TYPE}_${BUILD}
rm -rfv ${BUILD_OUTPUT_DIR}.tgz ${BUILD_OUTPUT_DIR}

if [ "$BUILD_TYPE" = 'release' ]
then
BUILD_DIR=release
else
BUILD_DIR=debug
fi

ISP_LIB_DIR=${BUILD_OUTPUT_DIR}/usr/lib
ISP_BIN_DIR=${BUILD_OUTPUT_DIR}/opt/imx8-isp/bin
ISP_BOOT_DIR=${BUILD_OUTPUT_DIR}/boot

mkdir -p ${ISP_LIB_DIR}
mkdir -p ${ISP_BIN_DIR}
mkdir -p ${ISP_BOOT_DIR}

cp -a appshell/build/generated/${BUILD_DIR}/lib/*.so* ${ISP_LIB_DIR}
cp -a appshell/build/generated/${BUILD_DIR}/bin/* ${ISP_BIN_DIR}
cp -ra mediacontrol/case/ ${ISP_BIN_DIR}
cp -a utils3rd/tools/ISP8000NANO_V1802/*  ${ISP_BIN_DIR}

chmod +x ${ISP_BIN_DIR}/run.sh
chmod +x ${ISP_BIN_DIR}/memtool

tar zcvf ${BUILD_OUTPUT_DIR}.tgz ${BUILD_OUTPUT_DIR}



