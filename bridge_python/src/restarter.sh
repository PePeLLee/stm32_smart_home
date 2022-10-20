#!/bin/sh

state=`ip link show can0 | grep -o "state [^ ]*"`

if [ "$state" = "state UP" ] || [ "$state" = "state UNKNOWN" ];
then
	echo "$state" 
else
	echo "$state" 
	echo 'restart' 
	sudo systemctl restart can2mqtt.service

fi
