# How to use the clock

## Setting the time:
1. Hold H/Hour button down for 1+ seconds to enter Set Time mode (The H button is the closest to the USB port)
2. Digit colors will change and the seconds will stop counting
3. Use H and M buttons to set the correct Hours and Minutes -- Holding M will advance minutes quickly. Note that H does not have the same fast functionality.
4. Hold H button down again for 1+ seconds to return to normal mode - the seconds will resume counting and the clock color will change back to the previous setting.

## Changing the brightness
Press the `B` button to set the brightness. There are 8 levels, and the current brightness is remembered when powering off the clock.

## Changing the color
The `C` button is typically used for changing the color of the digits, but its use depends on the animation mode the clock is currently using.

### Animation modes:
Animation modes can be changed by pressing the `M` button while not in `Set Time` mode. The current animation mode number will be displayed after each button press.

* ANIM_NONE (0): No animation, and pressing the C button will manually change the colors for all digits at once. The current color setting is saved when changed.
* ANIM_ZIPPY(1): An animation that causes movement through all the numbers every time a number changes.
* ANIM_GLOW (2): Similar to ANIM_NONE, except the digits briefly dim and glow every couple seconds. C button changes colors for all digits at once
* ANIM_CYCLE_COLORS (3): The colors will cycle across the digits as each second changes and the C button will not do much other than briefly flash a color.
* ANIM_CYCLE_FLOW_LEFT (4): The color of the furthest right digit will cycle each second and as each digit rolls from 9 back to 0, that digit's color will be copied to the left.
* ANIM_RAINBOW (5): All digits will continuously gently cycle colors while the clock is running.
* ... more modes are coming!

Changing the animation mode will be possible via the mobile app within the coming months, and can also be manually set via the  `setup()` function in `firmware.ino` and then [re-upload the firmware](INSTALLING.md).

## Toggling between edge-lit and PXL mode
Hold M+C for 1 second to toggle. This is useful when you switch between the acrylic digit base and the PXL cover.

## Toggling 12/24H mode
Hold H+M for 1 second.

## Flipping the display
Hold C+B for 1 second to toggle. This is useful if you want to put your digits in the reverse order, to keep the buttons at the front. Also, useful for PXL mode.
