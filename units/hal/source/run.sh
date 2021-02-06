export LD_LIBRARY_PATH=$pwd:$LD_LIBRARY_PATH
systemctl stop weston*

rmmod vvcam.ko
insmod vvcam.ko

./isp_media_server & ./video_test 1920 1080 0

