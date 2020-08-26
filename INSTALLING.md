# How to install/update the firmware

So it's time to update the firmware, for the first time, for the last time. Hopefully not the last time!

## Prepare your computer to talk to the RedBoard Nano
1. Install the Arduino IDE on your computer from https://www.arduino.cc/en/Main/Software. 
   At this moment, 1.8.13 is the latest version.
2. Open the Arduino Preferences menu (File->Preferences or Arduino-Preferences on macOS)
3. Enter `https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json` 
   into the Additional Boards Manager URLs -- this is to allow Arduino to download the RedBoard support in the following steps.
4. Click OK to close the Preferences window
5. Open `Tools->Board` and select `Boards Manager`
6. Search for `RedBoard Nano` to install the `SparkFun Apollo3 Boards` and select `Install`. 
   At this moment, 1.1.2 is the latest version of the Apollo3 Board support from SparkFun and is confirmed to work with the latest Foxie Clock firmware.
7. Select `Tools->Board->SparkFun RedBoard Artemis Nano`
8. Select `Tools->Programmer->Ambiq Secure Bootloader`
9. Make sure your Foxie Clock is plugged into your computer with a USB-C cable.
10. Select `Tools->Port` in Arduino and choose the correct port for your Foxie Clock. 
   On **most** platforms, this will have "wch" at the beginning of the serial port name.
   If there is more than one available port, you can unplug the clock, open `Tools->Port` to see what is there,
   then plug in the clock again and select the newly available port. 
   
   Note: On some operating systems (such as macOS), you may need to install WCH's CH340 drivers and **reboot** for the board support
   to detect the correct serial port. 
   See [SparkFun's How to Install CH340 Drivers](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers)
   for more information.
   
11. **OPTIONAL, but highly recommended:** Select `Tools->Burn Bootloader` to write the latest version of SparkFun's bootloader 
   to the Nano -- this is still important for the Artemis boards for Arduino to be able to write the firmware reliably.

### For more help with the RedBoard Nano:
[SparkFun's Hookup Guide for the SparkFun RedBoard Artemis Nano](https://learn.sparkfun.com/tutorials/hookup-guide-for-the-sparkfun-redboard-artemis-nano)

[SparkFun's Artemis Development with Arduino](https://learn.sparkfun.com/tutorials/artemis-development-with-arduino)

## Installing/updating the latest firmware
The best thing is to `git clone https://github.com/afoxinsocks/foxie-clock` to your local computer, then you can simply `git pull` to get the latest changes at any time.
1. In Arduino IDE, open `foxie-clock/firmware/firmware.ino`
2. Make some changes, if you want!
3. Select `Sketch->Upload` 
4. Remember, you can always undo any local changes easily with git in case you really mess it up. Don't be afraid to try new things!
