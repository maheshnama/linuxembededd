#!/bin/sh

if ps -ef | grep -v grep | grep taxiMeter; then
	exit 0
else
	echo "taxiMeter is restarting ..."
	#/root/TaxiMeter/taxiMeter >> /var/www/html/hawkeye.qdnet.com/taximeter/error.log
	/root/TaxiMeter/taxiMeter >>taxi.txt
	exit 0
fi

