# How to use the clock

## Setting the time:
1. Hold `H` button down for 1+ seconds to enter set time mode (The H button is the closest to the USB port)
2. Digit colors will change and the seconds will stop counting
3. Press `H` and `M` to set the correct Hours and Minutes
4. *Quietly wish that the mobile app to do this (and more) via Bluetooth was already done*
5. Hold H button down again for 1+ seconds to return to normal mode - the seconds will resume counting.

## Changing the brightness
Press the `B` button to set the brightness. There are 8 levels, and the current brightness is remembered when powering off the clock.

## Changing the color
The `C` button is typically used for changing the color of the digits, but its use depends on the animation mode the clock is currently using.

### Animation modes:
* ANIM_NONE: No animation, and pressing the `C` button will manually change the colors for all digits at once. The current color setting is saved when changed.
* ANIM_CYCLE_COLORS (**DEFAULT**): The colors will cycle across the digits as each second changes and the `C` button will not do much other than briefly flash a color.
* ANIM_GLOW: Similar to ANIM_NONE, except the digits briefly dim and glow every couple seconds. `C` button changes colors for all digits at once
* ... more modes are coming!

Changing the animation mode will be supported via a button sequence soon, and via the mobile app after that. Until then, to change the animation mode, please edit the `setup()` function in `firmware.ino` and then [re-upload the firmware](INSTALLING.md).
