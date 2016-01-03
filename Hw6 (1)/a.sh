rm /dev/MorseToLed
rmmod morse_module.ko
make
insmod morse_module.ko
mknod /dev/MorseToLed c 243 0
