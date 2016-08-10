# OCOLORUM (wip)
an abstract light clock running on arduino uno + rtc to drive neopixels into different color modes.

**hardware**

- arduino uno or attiny85/84
- DS1307, DS32331 rtc module
- NeoPixels (at least 10)

**dependencies**

* Wire.h (ATMega328p) or TinyWireM.h (https://github.com/adafruit/TinyWireM)


* Adafruit_NeoPixel.h
* Analog MultiButton (http://damienclarke.me/code/analog-multi-button)

**how does it work?**

a given array of 12 colors represents 11 gradients (inbetween) which are gradually tested over the running of a day.

e.g the first gradient starts midnight (0:00) and goes 'til 1:00. the delta between corresponding r, g and b values are being calculated, divided into steps and fed to a resolution of 3600 seconds (per hour). then over time gradually the colors will change almost unnoticably - unless you have a really good eye.

so 0:00 is in the array position 0 and fades into 1, 1:00 is position 1 and goes to position 2 and so on. around noon there is a change where the order gets reversed.

the gradients derive from the colors in the sky: from dusk til high noon dawn.