#!/usr/bin/env bash

UDID=$(system_profiler SPUSBDataType | sed -n -E -e '/(iPhone|iPad)/,/Serial/s/ *Serial Number: *(.+)/\1/p')
echo $UDID
PORT=12345
RESOLUTION="400x600"
UDID=DJH4291233MF6VTD5

./build/ios_minicap \
    --udid $UDID \
    --port $PORT \
    --resolution $RESOLUTION
