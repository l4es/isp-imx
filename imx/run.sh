#!/bin/bash
#
# Start the isp_media_server in the configuration from user
# (c) NXP 2020
# (c) Basler 2020
#

RUN_SCRIPT=`realpath -s $0`
RUN_SCRIPT_PATH=`dirname $RUN_SCRIPT`
echo "RUN_SCRIPT=$RUN_SCRIPT"
echo "RUN_SCRIPT_PATH=$RUN_SCRIPT_PATH"

LOAD_MODULES="0" # do not load modules, they are automatically loaded if present in /lib/modules
LOCAL_RUN="0" # search modules in /lib/modules and libraries in /usr/lib
JSON_FILE=""
LOCAL_RUN="0"
# an array with the modules to load, insertion order
declare -a MODULES=("imx8-media-dev" "vvcam-video" "vvcam-dwe" "vvcam-isp")

USAGE="Usage:\n"
USAGE+="run.sh -c <isp_config> &\n"
USAGE+="Supported configurations:\n"
USAGE+="\tbasler_1080p60    - single basler camera on MIPI-CSI1, 1920x1080, 60 fps\n"
USAGE+="\tbasler_4k         - single basler camera on MIPI-CSI1, 3840x2160, 30 fps\n"
USAGE+="\tbasler_1080p60hdr - single basler camera on MIPI-CSI1, 1920x1080, 60 fps, HDR configuration\n"
USAGE+="\tbasler_4khdr      - single basler camera on MIPI-CSI1, 3840x2160, 30 fps, HDR configuration\n"


# parse command line arguments
while [ "$1" != "" ]; do
	case $1 in
		-c | --config )
			shift
			ISP_CONFIG=$1
			;;
		-l | --local )
			LOCAL_RUN="1"
			# search modules and libraries near this script
			# this should work with the flat structure from VSI/Basler
			# but also with the output from make_isp_build_*.sh
			;;
		-lm | --load-modules )
			LOAD_MODULES="1"
			;;
		* )
			echo -e "$USAGE" >&2
			exit 1
	esac
	shift
done

write_default_mode_files () {
	# OV2775 modes file 
	echo -n "" > OV2775_MODES.txt
	echo "[sensor_mode.0]" >> OV2775_MODES.txt
	echo "xml = \"OV2775.xml\"" >> OV2775_MODES.txt
	echo "[sensor_mode.1]" >> OV2775_MODES.txt
	echo "xml = \"OV2775.xml\"" >> OV2775_MODES.txt
	echo "[sensor_mode.2]" >> OV2775_MODES.txt
	echo "xml = \"OV2775.xml\"" >> OV2775_MODES.txt
	echo "[sensor_mode.3]" >> OV2775_MODES.txt
	echo "xml = \"OV2775_8M_02_720p.xml\"" >> OV2775_MODES.txt

	# OS08A20 modes file - only mode 2 is supported 
	echo -n "" > OS08A20_MODES.txt
	echo "[sensor_mode.0]" >> OS08A20_MODES.txt
	echo "xml = \"OS08a20_4k.xml\"" >> OS08A20_MODES.txt
	echo "[sensor_mode.1]" >> OS08A20_MODES.txt
	echo "xml = \"OS08a20_4k.xml\"" >> OS08A20_MODES.txt
	echo "[sensor_mode.2]" >> OS08A20_MODES.txt
	echo "xml = \"OS08a20_4k.xml\"" >> OS08A20_MODES.txt
	echo "[sensor_mode.3]" >> OS08A20_MODES.txt
	echo "xml = \"OS08a20_4k.xml\"" >> OS08A20_MODES.txt

	# Basler DAA3840 modes file 
	echo -n "" > DAA3840_MODES.txt
	echo "[sensor_mode.0]" >> DAA3840_MODES.txt
	echo "xml = \"DAA3840_30MC_4K.xml\"" >> DAA3840_MODES.txt
	echo "[sensor_mode.1]" >> DAA3840_MODES.txt
	echo "xml = \"DAA3840_30MC_1080P.xml\"" >> DAA3840_MODES.txt
	echo "[sensor_mode.2]" >> DAA3840_MODES.txt
	echo "xml = \"DAA3840_30MC_4K.xml\"" >> DAA3840_MODES.txt
	echo "[sensor_mode.3]" >> DAA3840_MODES.txt
	echo "xml = \"DAA3840_30MC_1080P.xml\"" >> DAA3840_MODES.txt
}

# write the sensonr config file
write_sensor_cfg_file () {
	local SENSOR_FILE="$1"
	local CAM_NAME="$2"
	local DRV_FILE="$3"
	#local XML_FILE="$4"
	local MODE_FILE="$4"
	local MODE="$5"

	echo -n "" > $SENSOR_FILE
	echo "name = \"$CAM_NAME\"" >> $SENSOR_FILE
	echo "drv = \"$DRV_FILE\"" >> $SENSOR_FILE
	#echo "xml = \"$XML_FILE\"" >> $SENSOR_FILE
	echo "mode = $MODE" >> $SENSOR_FILE
	cat $MODE_FILE >> $SENSOR_FILE

	if [ ! -f $DRV_FILE ]; then
		echo "File does not exist: $DRV_FILE" >&2
		exit 1
	fi
	if [ ! -f $MODE_FILE ]; then
		echo "File does not exist: $MODE_FILE" >&2
		exit 1
	fi
}

# write the sensonr config file
set_libs_path () {
	local ONE_LIB="$1"
	LIB_PATH=`find $RUN_SCRIPT_PATH -name $ONE_LIB | head -1`
	if [ ! -f "$LIB_PATH" ]; then
		LIB_PATH=`find $RUN_SCRIPT_PATH/../../../usr -name $ONE_LIB | head -1`
		if [ ! -f "$LIB_PATH" ]; then
			echo "$ONE_LIB not found in $RUN_SCRIPT_PATH"
			echo "$ONE_LIB not found in $RUN_SCRIPT_PATH/../../../usr"
			exit 1
		fi
	fi
	LIB_PATH=`realpath $LIB_PATH`
	export LD_LIBRARY_PATH=`dirname $LIB_PATH`:/usr/lib:$LD_LIBRARY_PATH
	echo "LD_LIBRARY_PATH set to $LD_LIBRARY_PATH"
}

load_module () {
	local MODULE="$1.ko"
	local MODULE_PARAMS="$2"

	# return directly if already loaded.
	MODULENAME=`echo $1 | sed 's/-/_/g'`
	echo $MODULENAME
	if lsmod | grep "$MODULENAME" ; then
		echo "$1 already loaded."
		return 0
	fi

	if [ "$LOCAL_RUN" == "1" ]; then
		MODULE_SEARCH=$RUN_SCRIPT_PATH
		MODULE_PATH=`find $MODULE_SEARCH -name $MODULE | head -1`
		if [ "$MODULE_PATH" == "" ]; then
			MODULE_SEARCH=$RUN_SCRIPT_PATH/../../../modules
			MODULE_PATH=`find $MODULE_SEARCH -name $MODULE | head -1`
			if [ "$MODULE_PATH" == "" ]; then
				echo "Module $MODULE not found in $RUN_SCRIPT_PATH"
				echo "Module $MODULE not found in $MODULE_SEARCH"
				exit 1
			fi
		fi
		MODULE_PATH=`realpath $MODULE_PATH`
	else
		MODULE_SEARCH=/lib/modules/`uname -r`
		MODULE_PATH=`find $MODULE_SEARCH -name $MODULE | head -1`
		if [ "$MODULE_PATH" == "" ]; then
			echo "Module $MODULE not found in $MODULE_SEARCH"
			exit 1
		fi
		MODULE_PATH=`realpath $MODULE_PATH`
	fi
	insmod $MODULE_PATH $MODULE_PARAMS  || exit 1
	echo "Loaded $MODULE_PATH $MODULE_PARAMS"
}

load_modules () {
	# remove any previous instances of the modules
	n=${#MODULES_TO_REMOVE[*]}
	for (( i = n-1; i >= 0; i-- ))
	do
		echo "Removing ${MODULES_TO_REMOVE[i]}..."
		rmmod ${MODULES_TO_REMOVE[i]} &>/dev/null
		#LSMOD_STATUS=`lsmod | grep "${MODULES[i]}"`
		#echo "LSMOD_STATUS=$LSMOD_STATUS"
		if lsmod | grep "${MODULES_TO_REMOVE[i]}" ; then
			echo "Removing ${MODULES_TO_REMOVE[i]} failed!"
			exit 1
		fi
	done

	# and now clean load the modules
	for i in "${MODULES[@]}"
	do
		echo "Loading module $i ..."
		load_module $i
	done
}

write_default_mode_files

echo "Trying configuration \"$ISP_CONFIG\"..."
MODULES_TO_REMOVE=( "basler-camera-driver-vvcam" "os08a20" "ov2775" "${MODULES[@]}")
case "$ISP_CONFIG" in
		basler_4k )
			MODULES=("basler-camera-driver-vvcam" "${MODULES[@]}")
			JSON_FILE="case/config/daA3840_30mc_4K.json"
			CAM_NAME="basler-vvcam"
			DRV_FILE="DAA3840_30MC_4K.drv"
			#XML_FILE="DAA3840_30MC_4K.xml"
			MODE_FILE="DAA3840_MODES.txt"
			MODE="0"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			VVCAM_PARAMS="fourcc=$FOURCC width=3840 height=2160"
			;;
		basler_1080p60 )
			MODULES=("basler-camera-driver-vvcam" "${MODULES[@]}")
			JSON_FILE="case/config/daA3840_30mc_1080P.json"
			CAM_NAME="basler-vvcam"
			DRV_FILE="DAA3840_30MC_1080P.drv"
			#XML_FILE="DAA3840_30MC_1080P.xml"
			MODE_FILE="DAA3840_MODES.txt"
			MODE="1"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			VVCAM_PARAMS="fourcc=$FOURCC width=1920 height=1080"
			;;
		basler_4khdr )
			MODULES=("basler-camera-driver-vvcam" "${MODULES[@]}")
			JSON_FILE="case/config/daA3840_30mc_4K.json"
			CAM_NAME="basler-vvcam"
			DRV_FILE="DAA3840_30MC_4K.drv"
			#XML_FILE="DAA3840_30MC_4K.xml"
			MODE_FILE="DAA3840_MODES.txt"
			MODE="2"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			VVCAM_PARAMS="fourcc=$FOURCC width=3840 height=2160"
			;;
		basler_1080p60hdr )
			MODULES=("basler-camera-driver-vvcam" "${MODULES[@]}")
			JSON_FILE="case/config/daA3840_30mc_1080P.json"
			CAM_NAME="basler-vvcam"
			DRV_FILE="DAA3840_30MC_1080P.drv"
			#XML_FILE="DAA3840_30MC_1080P.xml"
			MODE_FILE="DAA3840_MODES.txt"
			MODE="3"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			VVCAM_PARAMS="fourcc=$FOURCC width=1920 height=1080"
			;;
		ov2775_1080p30 )
			MODULES=("ov2775" "${MODULES[@]}")
			JSON_FILE="case/config/2006_sensor_dwe.json"
			CAM_NAME="ov2775"
			DRV_FILE="ov2775.drv"
			#XML_FILE="OV2775.xml"
			MODE_FILE="OV2775_MODES.txt"
			MODE="0"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			;;
		ov2775_1080p30hdr )
			MODULES=("ov2775" "${MODULES[@]}")
			JSON_FILE="case/config/2006_sensor_dwe.json"
			CAM_NAME="ov2775"
			DRV_FILE="ov2775.drv"
			#XML_FILE="OV2775.xml"
			MODE_FILE="OV2775_MODES.txt"
			MODE="1"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			;;
		ov2775_1080p30hdr_native )
			MODULES=("ov2775" "${MODULES[@]}")
			JSON_FILE="case/config/2006_sensor_dwe.json"
			CAM_NAME="ov2775"
			DRV_FILE="ov2775.drv"
#			XML_FILE="OV2775.xml"
			MODE_FILE="OV2775_MODES.txt"
			MODE="2"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			;;
		ov2775_720p )
			MODULES=("ov2775" "${MODULES[@]}")
			JSON_FILE="case/config/2006_sensor_dwe.json"
			CAM_NAME="ov2775"
			DRV_FILE="ov2775.drv"
			#XML_FILE="OV2775.xml"
			MODE_FILE="OV2775_MODES.txt"
			MODE="3"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			;;
		os08a20_4k )
			MODULES=("os08a20" "${MODULES[@]}")
			JSON_FILE="case/config/2005_mc_sensor.json"
			CAM_NAME="os08a20"
			DRV_FILE="os08a20.drv"
			#XML_FILE="OS08a20_4k.xml"
			MODE_FILE="OS08A20_MODES.txt"
			MODE="2"
			write_sensor_cfg_file "Sensor0_Entry.cfg" $CAM_NAME $DRV_FILE $MODE_FILE $MODE
			;;
		*)
			echo "ISP configuration \"$ISP_CONFIG\" unsupported."
			echo -e "$USAGE" >&2
			exit 1
			;;
	esac

if [ ! -f $JSON_FILE ]; then
	echo "Configuration file does not exist: $JSON_FILE" >&2
	exit 1
fi

PIDS_TO_KILL=`pgrep -f video_test\|isp_media_server`
if [ ! -z "$PIDS_TO_KILL" ]
then
	echo "Killing preexisting instances of video_test and isp_media_server:"
	echo `ps $PIDS_TO_KILL`
	pkill -f video_test\|isp_media_server
fi

# Need a sure way to wait untill all the above processes terminated
sleep 1

if [ "$LOAD_MODULES" == "1" ]; then
	load_modules
fi

if [ "$LOCAL_RUN" == "1" ]; then
	set_libs_path "libmedia_server.so"
fi

echo "Starting isp_media_server with configuration file $JSON_FILE"
./isp_media_server $JSON_FILE

# this should now work
# gst-launch-1.0 -v v4l2src device=/dev/video0 ! "video/x-raw,format=YUY2,width=1920,height=1080" ! queue ! imxvideoconvert_g2d ! waylandsink
# gst-launch-1.0 -v v4l2src device=/dev/video0 ! "video/x-raw,format=YUY2,width=3840,height=2160" ! queue ! imxvideoconvert_g2d ! waylandsink
# gst-launch-1.0 -v v4l2src device=/dev/video0 ! waylandsink
