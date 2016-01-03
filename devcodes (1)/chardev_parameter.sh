#!/bin/bash

read -p "Enter the number to add " number
echo $number > /sys/module/chardev/parameters/a
cat /dev/chardev

