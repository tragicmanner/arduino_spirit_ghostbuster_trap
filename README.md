# arduino_spirit_ghostbuster_trap
Code to drive the Spirit Halloween Ghostbuster ghost trap that has been modded with Servos and/or additional LEDs.

## Requirements
This code requires the [Coroutines Library](https://github.com/renaudbedard/littlebits-arduino/tree/master/Libraries/Coroutines) by Renaudbedard to run properly. Just grab the Coroutines.h file and put it in a folder called Coroutines inside your Arduino/libraries folder. See [this article on Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries#toc5) for more details.

It also requires the YX5300 library I put together in this repo in the YX5300 folder. Just copy that folder in your Arduino/libraries directory.

This also requires a Arduino Uno or Arduino Nano to be run in its current state, but the code can easily be adapted to run on all sorts of arduino and arduino compatible devices.

Also, as an additional note, while this code is written to run 2 servos and 6 LEDs, none of these are required for the code to run. If something is not hooked up to a pin that is used, it will not keep other parts of the code from running. So feel free to attach as many or as few of the devices as you wish.

## Servos
The code supports two servos out of the box, which need to be added to your SH (spirit halloween) ghost trap. For details on how I have the servo modded into my trap, see [this youtube video](https://youtu.be/5K85QkGYRnw)

The code has angles set up for my servos, but you will need to modify them for your servos and how you have them set up. EVERY SERVO WILL BE DIFFERENT. YOU MUST MODIFY THE VARIABLES servo1OpenPosition, servo1ClosedPosition, servo2OpenPosition, and servo2ClosedPosition so that the code positions your servos properly to open and close your trap without binding or other issues.

You are more than welcome to change the servo pins, just keep in mind that you want to use a PWM pin, and make sure that LED pins are not assigned to the same pin.

## LEDs
The code supports six LEDs out of the box. Two Non-PWM LEDs that just turn on and off, and four PWM LEDs that can fade and have more effects when being run.

PWMLEDs is an array with which pins PWM LEDs are hooked up to. nonPWMLEDs is an array of the LEDs that will just turn on and off. You can easily add non-PWM LEDs by adding their pins to the nonPWMLEDs array, just make sure you size it appropriately. For example, if I wanted to add another LED on pin 4, I would change the line to:
`int nonPWMLEDs[3] = {7,8,4};`

Adding additional LEDs for the PWMLEDs is a little bit more involved. First you will need to add the LED to the `PWMLEDs` array, also making sure to resize it properly, but then you will also need to add entries to the `brightness` and `fadeAmount` arrays. Each LED needs to have these values set to control their behavior, and without the code will currently fail.

In other words, make sure that `PWMLEDs`, `brightness` and `fadeAmount` are arrays with the same length.

## Coroutines
This code makes heavy use of cortoutines to run things smoothly, and the coroutines can easily be modified to do additional tasks as necessary. Just add function calls that you would like to include or remove ones that you don't want to run to the OpenDoors or CloseDoors coroutines to change behavior of what happens at trap opening and closing.

## Additional Thoughts
I appreciate feedback, and feel free to drop into my thread on my SH Trap Mod at [gbfans.com](https://www.gbfans.com/forum/viewtopic.php?f=3&t=44674) to provide any suggestions or ask for any help you may need.