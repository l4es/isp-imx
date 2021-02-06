export LD_LIBRARY_PATH=$pwd:$LD_LIBRARY_PATH
ps -ef | grep video_test | grep -v grep | awk '{print $2}' |  xargs kill -9
ps -ef | grep isp_media_server | grep -v grep | awk '{print $2}' |  xargs kill -9

#systemctl stop weston*

rmmod vvcam-video
rmmod vvcam-isp
rmmod vvcam-dwe
rmmod vvcam-csis
rmmod ov2775

insmod vvcam-video.ko
insmod vvcam-isp.ko
insmod vvcam-dwe.ko
insmod vvcam-csis.ko
insmod ov2775.ko

./isp_media_server case/config/2006_sensor_basler.json & gst-launch-1.0 -v v4l2src device=/dev/video0 ! "video/x-raw,format=YUY2,width=1920,height=1080" ! queue ! imxvideoconvert_g2d !  waylandsink

