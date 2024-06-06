#!/bin/bash

DIR_LOG=/data4/e1039_data/online/decoder_maindaq
FN_LOG_ROT=log_rotate_log.txt

while true ; do
    echo "----------------"
    while read FN_LOG ; do
	echo "FN_LOG: $FN_LOG"
	gzip $FN_LOG
    done < <(find $DIR_LOG -name 'log_[0-9][0-9][0-9][0-9][0-9][0-9].txt*' -mtime +1)

    while read FN_LOG_GZ ; do
	echo "FN_LOG_GZ: $FN_LOG_GZ"
	#rm $FN_LOG_GZ
    done < <(find $DIR_LOG -name 'log_[0-9][0-9][0-9][0-9][0-9][0-9].txt*.gz' -mtime +14)

    sleep 600
done &>$DIR_LOG/$FN_LOG_ROT
