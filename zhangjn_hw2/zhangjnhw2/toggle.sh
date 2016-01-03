#!/usr/bin/bash

cd /
cd /sys/class/gpio
cd gpio17
V=`cat value`
if test "$V" == '1'
then echo '0' > value
fi

if test "$V" == '0'
then echo '1' > value
fi

#cat value
