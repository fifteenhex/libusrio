# libusrio
Library for using the Linux i2cdev/spidev style API but in userspace for
stuff like crappy USB IO dongle thingys that might or might not have in
kernel drivers.

The idea here is that you can write code that works with an in-kernel
i2c/spi driver and with a few simple changes have it work with some
usb dongle thingy for easier testing on your desktop PC or whatever.
