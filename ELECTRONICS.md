# Electronics

Time to dust off your soldering skills, we've just got a few buttons to solder and finally need to attach the RedBoard Nano to the Foxie Clock PCB. 

![Parts](/images/assembly_tools.jpg)

Included parts:
* One 2-pin header
* One 5-pin header
* 4 buttons
* One RedBoard Nano
* One "assembly helper"

**Note:** The clippers are necessary to trim the tops of the button pins, otherwise the lid will not sit correctly.

![Parts](/images/assembly_step0.jpg)

## Step 1

Insert the 4 buttons into the holes on the bottom of the PCB.

![](/images/assembly_step1.jpg)

## Step 2

Ensure that the buttons are completely flat against the PCB.

![](/images/assembly_step2.jpg)

## Step 3

Turn the board over with the buttons in it. They are a snug fit and **should** stay in place.

![](/images/assembly_step3.jpg)

## Step 4

Begin by soldering one of the pins for each button. After doing so, double check that the buttons are still right against the PCB -- the tolerances on the back of the 3d printed case require these buttons to be exactly flush to the PCB. 

![](/images/assembly_step4.jpg)

## Step 5

After verifying (and resoldering, if necessary) the single pin holding each of the buttons next to the PCB, go ahead and solder the other 3 pins for each button.

![](/images/assembly_step5.jpg)

## Step 6

Prepare the RedBoard Nano by inserting the two headers into the *bottom* of the RedBoard (the side without the USB connector). The 2-pin header goes into the VIN and GND holes. The 5-pin header goes into the holes labled `6 A5 4 A3 A2`. **Do not solder yet!**

**NOTE**: Make sure the long pins of the jumpers are going through the RedBoard to the "top" side, otherwise you'll have to clip them off later when we trim the button pins.

![](/images/assembly_step6.jpg)

## Step 7

Holding the Foxie Clock PCB vertically, carefully insert the pins from the RedBoard into the Foxie Clock board, then set it down.

![](/images/assembly_step7a.jpg)
![](/images/assembly_step7b.jpg)
![](/images/assembly_step7c.jpg)

## Step 8

Attach the Assembly helper to hold the RedBoard Nano to the PCB so that it is properly positioned. As with the buttons, it is important that the RedBoard be correctly positioned **before** soldering so that the USB port is in the correct place for
the USB port hole in the back of the case.

![](/images/assembly_step8a.jpg)
![](/images/assembly_step8b.jpg)
![](/images/assembly_step8c.jpg)

## Step 9

Solder the 2-pin and 5-pin headers on the bottom of the RedBoard.

![](/images/assembly_step9a.jpg)
![](/images/assembly_step9b.jpg)

## Step 10

Carefully turn the board over, keeping the assembly helper attached, and solder the 2-pin and 5-pin headers to the top of the PCB.

![](/images/assembly_step10a.jpg)
![](/images/assembly_step10b.jpg)

## Step 11

Remove the assembly helper, and marvel at your perfectly placed RedBoard Nano on 7 little stilts.

![](/images/assembly_step11.jpg)

## Step 12

Trim the excess height of the button leads. 

**NOTE:** It is not important to get them flush. They just need to be trimmed to be about the same height as the LEDs, so that the lid sits correctly inside the case.

**NOTE:** It's a good idea to reflow the solder on these joints after being trimmed.

![](/images/assembly_step12a.jpg)
![](/images/assembly_step12b.jpg)

## Step 13

Congratulations, the electronics are ready to go into the case!

Begin by placing the PCB in the rear top corners of the case.
![](/images/assembly_step13a.jpg)

Carefully push the buttons into the button holes.
![](/images/assembly_step13b.jpg)

The board should drop straight in at this point, if the buttons and Nano are close to being correctly placed.
![](/images/assembly_step13c.jpg)

Make sure that the board is completely flush with the inside back of the case.
![](/images/assembly_step13d.jpg)
![](/images/assembly_step13e.jpg)

You can now put the lid on top of the PCB and insert the 4 bolts, if desired. Make sure not to overtighten the bolts.

## Step 14

[Use Arduino to program the firmware onto your Foxie Clock](INSTALLING.md)

[Continue onto the rest of the assembly](ASSEMBLY.md)
