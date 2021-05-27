Base folder contains the code for dongle base\
Each thingy folder contains the code for the thingy52 attached to the following bodypart:
* thingy0: head
* thingy1: wrist
* thingy2: leg

Required OS:\
zephyr

Hardware connection set up:
* Dongle: dongle can be plug into PC through USB port.
* Thingy52: There is a switch button on the side of the thing52, you need to switch the button from "o" to "I" to open the device.

Build code for base:
```
>>> west build -p -b nrf52840dongle_nrf52840 base
>>> nrfutil pkg generate --hw-version 52 --sd-req=0x00         --application build/zephyr/zephyr.hex         --application-version 1 flash.zip
>>> sudo chmod 666 /dev/ttyACM0
>>> nrfutil dfu usb-serial -pkg flash.zip -p /dev/ttyACM0
```

Build code for thingy52 (X as 0, 1 or 2):
```
>>> west build -p -b thingy52_nrf52832 thingyX
>>> west flash
```

After building the code, data can be received through the device. Python will do the data pre-processing and then it goes to KNN algorithm. Window time is 3s and it will calucalte 15 data in one time.



