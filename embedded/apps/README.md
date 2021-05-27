Base folder contains the code for dongle base\
Each thingy folder contains the code for the thingy52 attached to the following bodypart:
* thingy0: head
* thingy1: wrist
* thingy2: leg


Build code for base:
```
>>> west build -p -b nrf52840dongle_nrf52840 base
>>> nrfutil pkg generate --hw-version 52 --sd-req=0x00         --application build/zephyr/zephyr.hex         --application-version 1 flash.zip
>>> sudo chmod 666 /dev/ttyACM0
>>> nrfutil dfu usb-serial -pkg flash.zip -p /dev/ttyACM0
```

