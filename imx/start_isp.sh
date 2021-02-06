#!/bin/sh
#
# Start the isp_media_server in the configuration for Basler daA3840-30mc
#
# (c) Basler 2020
# (c) NXP 2020
#

RUNTIME_DIR="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
DEVICE_TREE_BASLER=$(grep basler-camera-vvcam /sys/firmware/devicetree/base/soc@0/*/i2c@*/*/compatible -l 2> /dev/null)


# check if the basler device has been enabled in the device tree
if [ -f "$DEVICE_TREE_BASLER" ]; then

	echo "Starting isp_media_server for Basler daA3840-30mc"

	cd $RUNTIME_DIR
	# Default configuration for Basler daA3840-30mc: basler_4k
	# Available configurations: basler_4k, basler_1080p60, basler_4khdr, basler_1080p60hdr
	exec ./run.sh -c basler_4k -lm

else
	# no device tree found exit with code no device or address
	echo "No device tree found for Basler, check dtb file!" >&2
	exit 6
fi
