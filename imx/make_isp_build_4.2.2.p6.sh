#!/bin/sh
#
# Create a local ISP build from scratch
#

# ----------------------------------------------------------
# INPUTS_BEGIN
# ----------------------------------------------------------
ISP_IMX_GIT="ssh://git@bitbucket.sw.nxp.com/imxpriv/verisilicon_sw_isp.git"
# https://bitbucket.sw.nxp.com/projects/IMXPRIV/repos/verisilicon_sw_isp
#ISP_IMX_BRANCH="integration_20201127_4.2.2_p6"
ISP_IMX_BRANCH="partial_release_20201127_4.2.2_p6"

ISP_VVCAM_GIT="ssh://bitbucket.sw.nxp.com/imx/verisilicon_sw_isp_vvcam.git"
# https://bitbucket.sw.nxp.com/projects/IMXPRIV/repos/verisilicon_sw_isp_vvcam
ISP_VVCAM_BRANCH="imx_5.4.70_2.3.0"

BASLER_CAMERA_GIT="ssh://git@bitbucket.sw.nxp.com/imxpriv/basler_sw_camera.git"
# https://bitbucket.sw.nxp.com/projects/IMXPRIV/repos/basler_sw_camera
#BASLER_CAMERA_BRANCH="daA3840_30mc_NXP_4.2.2p5_312351_20201111-imx8mp_v3.2.1"
#BASLER_CAMERA_BRANCH="daA3840_30mc_NXP_4.2.2p5_312351_20201111-imx8mp_v3.2.1_patch20.11.2020"
BASLER_CAMERA_BRANCH="daA3840_30mc_NXP_4.2.2P6_patch_20201127-imx8mp_v3.2.5"


BUILD_TYPE="release" # release | debug
BUILD_FULL="partial" # full | partial
export VSI_LOG_LEVEL=1
GIT_CHECKOUT=1 # 0=use existing sources | 1=clone&checkout or fetch&reset

WORKING_DIR="`pwd`/local_isp_build"

# ----------------------------------------------------------
# INPUTS_END
# ----------------------------------------------------------

SDK_PATH="/opt/fsl-imx-xwayland/5.4-zeus/environment-setup-aarch64-poky-linux"
KERNEL_SOURCE_DIR=/work/linux-imx

HOST_CFG="`pwd`/`hostname`.cfg"
if [ -f $HOST_CFG ]; then
	. $HOST_CFG
	echo "Host configuration from $HOST_CFG"
fi
echo "SDK_PATH=$SDK_PATH"
echo "KERNEL_SOURCE_DIR=$KERNEL_SOURCE_DIR"
export KERNEL_SOURCE_DIR=$KERNEL_SOURCE_DIR
export KERNEL_SRC=$KERNEL_SOURCE_DIR

# print the above inputs and ask user to continue
THIS_FILE="$0"
awk '/^# INPUTS_BEGIN/,/^# INPUTS_END/' $THIS_FILE
echo ""
read -p "Using the above settings for the build, modify this script to change or press any keys to continue..."  KEYS

LOG_FILE=$WORKING_DIR/build.log


# Checkout the sources from git & branch
git_checkout_branch () {
	local GIT_REPO="$1"
	local GIT_BRANCH="$2"
	GIT_DIR="$WORKING_DIR/`basename $GIT_REPO`"
	GIT_DIR="${GIT_DIR%.*}"

	if [ $GIT_CHECKOUT -eq 0 ]
	then
		return
	fi

	if [ ! -d $GIT_DIR ]; then
		echo "$GIT_DIR not present" >&2
		echo "Cloning from repo: $GIT_REPO into $GIT_DIR"
		git clone $GIT_REPO
		echo "clone return $?"
	fi

	cd $GIT_DIR
	CURRENT_BRANCH=`git rev-parse --abbrev-ref HEAD`

	if [ $GIT_BRANCH = $CURRENT_BRANCH ]; then
		echo "$GIT_BRANCH already present, fetch and reset to origin"
		git fetch || { echo "git fetch failed in `pwd`"; exit 1; }
		git reset --hard "origin/$GIT_BRANCH" || exit 1
	else
		git fetch || { echo "git fetch failed in `pwd`"; exit 1; }
		echo "Hard reset Current branch: $CURRENT_BRANCH"
		git reset --hard "origin/$CURRENT_BRANCH" || exit 1
		echo "Checkout branch: $GIT_BRANCH"
		#git checkout -f $GIT_BRANCH || git checkout -b $GIT_BRANCH -t origin/$GIT_BRANCH || exit 1
		git checkout -f $GIT_BRANCH || exit 1
		git reset --hard "origin/$GIT_BRANCH" || exit 1
	fi
#	GIT_STATUS="`git status`"
#	if [ "$GIT_STATUS" != *"Your branch is up to date with"* ]; then
#		echo "git status not ok in $GIT_DIR"
#		exit 1
#	fi
	echo "Checkout finished in $GIT_DIR"
	cd - >/dev/null
}

# Checkout the sources from git & branch
git_checkout_tag () {
	local GIT_REPO="$1"
	local GIT_BRANCH="$2"
	GIT_DIR="$WORKING_DIR/`basename $GIT_REPO`"
	GIT_DIR="${GIT_DIR%.*}"

	if [ $GIT_CHECKOUT -eq 0 ]
	then
		return
	fi

	if [ ! -d $GIT_DIR ]; then
		echo "$GIT_DIR not present" >&2
		echo "Cloning from repo: $GIT_REPO into $GIT_DIR"
		git clone $GIT_REPO
		echo "clone return $?"
	fi

	cd $GIT_DIR
	CURRENT_BRANCH=`git rev-parse --abbrev-ref HEAD`

	if [ $GIT_BRANCH = $CURRENT_BRANCH ]; then
		echo "$GIT_BRANCH already present, fetch and reset to origin"
		git fetch || { echo "git fetch failed in `pwd`"; exit 1; }
		git reset --hard "$GIT_BRANCH" || exit 1
	else
		git fetch || { echo "git fetch failed in `pwd`"; exit 1; }
		echo "Checkout tag: $GIT_BRANCH"
		git checkout $GIT_BRANCH || exit 1
	fi
#	GIT_STATUS="`git status`"
#	if [ "$GIT_STATUS" != *"Your branch is up to date with"* ]; then
#		echo "git status not ok in $GIT_DIR"
#		exit 1
#	fi
	echo "Checkout finished in $GIT_DIR"
	cd - >/dev/null
}

# ----------------------------------------------------------
# get the sources
# ----------------------------------------------------------
	if [ ! -d $WORKING_DIR ]; then
		mkdir $WORKING_DIR || exit 1
	fi
	cd $WORKING_DIR || exit 1

	git_checkout_branch $ISP_IMX_GIT $ISP_IMX_BRANCH
	ISP_IMX_DIR=$GIT_DIR

	git_checkout_branch $ISP_VVCAM_GIT $ISP_VVCAM_BRANCH
	ISP_VVCAM_DIR=$GIT_DIR

	git_checkout_branch $BASLER_CAMERA_GIT $BASLER_CAMERA_BRANCH
	BASLER_CAMERA_DIR=$GIT_DIR

# ----------------------------------------------------------
# source the SDK
# ----------------------------------------------------------
export ARCH="arm64"
export CROSS_COMPILE="aarch64-linux-gnu-"
. $SDK_PATH

# ----------------------------------------------------------
# build ISP libs
# ----------------------------------------------------------
echo "Building ISP libs..."
if [ ! -d $ISP_IMX_DIR ]; then
	echo "$ISP_IMX_DIR does not exist, GIT_CHECKOUT=$GIT_CHECKOUT"
	exit 1
fi 
cd $ISP_IMX_DIR
#./build-all-isp.sh $BUILD_TYPE full >$LOG_FILE
./build-all-isp.sh $BUILD_TYPE $BUILD_FULL
echo "ISP libs build status: $?"
BUILD_OUTPUT_BASENAME="build_output_${BUILD_TYPE}_${BUILD_FULL}"
BUILD_OUTPUT=`pwd`/$BUILD_OUTPUT_BASENAME
ls $BUILD_OUTPUT
cd - >/dev/null

# ----------------------------------------------------------
# build vvcam
# ----------------------------------------------------------
cd $ISP_VVCAM_DIR ||  exit 1
# KERNEL_SOURCE_DIR/KERNEL_SRC set above
echo "Building vvcam with KERNEL_SOURCE_DIR=$KERNEL_SOURCE_DIR"
./build-all-vvcam.sh

cp -rf $ISP_VVCAM_DIR/modules $BUILD_OUTPUT/ || exit 1
ls -la $BUILD_OUTPUT/modules/*.ko
cd - >/dev/null

# ----------------------------------------------------------
# copy basler binaries
# ----------------------------------------------------------
cd $BASLER_CAMERA_DIR ||  exit 1
rm -rf build_output_basler
./copy-basler.sh $BUILD_TYPE build_output_basler
cp -rf build_output_basler/* $BUILD_OUTPUT/
cd - >/dev/null

# ----------------------------------------------------------
# copy scripts
# ----------------------------------------------------------
cp $ISP_IMX_DIR/imx/run.sh $BUILD_OUTPUT/opt/imx8-isp/bin/run.sh || exit 1
cp $ISP_IMX_DIR/imx/start_isp.sh $BUILD_OUTPUT/opt/imx8-isp/bin/ || exit 1
cp $ISP_IMX_DIR/imx/imx8-isp.service $BUILD_OUTPUT/opt/imx8-isp/bin/ || exit 1

# ----------------------------------------------------------
# copy kernel image & dtbs
# ----------------------------------------------------------
rm -rf $BUILD_OUTPUT/boot
mkdir $BUILD_OUTPUT/boot
cp $KERNEL_SOURCE_DIR/arch/arm64/boot/Image $BUILD_OUTPUT/boot || exit 1
cp $KERNEL_SOURCE_DIR/arch/arm64/boot/dts/freescale/imx8mp-evk*.dtb $BUILD_OUTPUT/boot || exit 1

cp $KERNEL_SOURCE_DIR/drivers/staging/media/imx/imx8-media-dev.ko $BUILD_OUTPUT/modules || exit 1

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
echo "${GREEN}Your build is ready in: ${BUILD_OUTPUT}${NOCOLOR}"
echo "Copy it on the target rootfs, let's say /home/root/$BUILD_OUTPUT_BASENAME then either:"
echo "${BLUE}A) Local run${NOCOLOR} in /home/root/$BUILD_OUTPUT_BASENAME/opt/imx8-isp/bin/"
echo "	systemctl stop imx8-isp"
echo "	./start_isp.sh &"
echo "${RED}B) Deployed service run${NOCOLOR}"
echo "	cd /home/root/$BUILD_OUTPUT_BASENAME"
echo "	./deploy.sh"
echo "	systemctl status imx8-isp"
if [ $GIT_CHECKOUT -eq 0 ]; then
echo "${RED}Please note this build was run without git sources update${NOCOLOR}"
fi

# now create the deploy script, to copy libs, binaries, modules to the right places
touch $BUILD_OUTPUT/deploy.sh
chmod a+x $BUILD_OUTPUT/deploy.sh
echo "#!/bin/sh" >> $BUILD_OUTPUT/deploy.sh
echo "cp -rf opt/imx8-isp/* /opt/imx8-isp/" >> $BUILD_OUTPUT/deploy.sh
echo "cp -rf usr/lib/* /usr/lib" >> $BUILD_OUTPUT/deploy.sh

#echo "cp modules/*.ko /lib/modules/\`uname -r\`/extra" >> $BUILD_OUTPUT/deploy.sh
MOD_PATH="modules/vvcam-csis.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra/csi/samsung"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/imx8-media-dev.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/kernel/drivers/staging/media/imx"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/vvcam-video.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra/video"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/vvcam-isp.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/vvcam-dwe.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/basler-camera-driver-vvcam.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra/sensor/camera-proxy-driver"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/ov2775.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra/sensor/ov2775"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

MOD_PATH="modules/os08a20.ko"
DEPLOY_PATH="/lib/modules/\`uname -r\`/extra/sensor/os08a20"
echo "mkdir -p $DEPLOY_PATH && cp $MOD_PATH $DEPLOY_PATH" >> $BUILD_OUTPUT/deploy.sh

echo "echo \"The following modules are installed:\"" >> $BUILD_OUTPUT/deploy.sh
echo "find /lib/modules/\`uname -r\`/extra -name *.ko" >> $BUILD_OUTPUT/deploy.sh

echo "systemctl stop imx8-isp>/dev/null" >> $BUILD_OUTPUT/deploy.sh
echo "cp opt/imx8-isp/bin/imx8-isp.service  /etc/systemd/system/" >> $BUILD_OUTPUT/deploy.sh
echo "systemctl enable imx8-isp" >> $BUILD_OUTPUT/deploy.sh
echo "systemctl start imx8-isp>/dev/null" >> $BUILD_OUTPUT/deploy.sh
