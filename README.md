# aburndance

rainbow leds you can dance with!

in the classic shape of suspenders and a belt.

## bom

bill of materials:

- led strips: [apa102](https://www.adafruit.com/product/2239?length=2)
  - [connector](https://www.amazon.com/gp/product/B0777BQC1P/)
- controller: [Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)
  - [Feather proto board](https://www.adafruit.com/product/2884)
  - [rotary encoder](https://www.adafruit.com/product/377)
  - [tactile button](https://www.adafruit.com/product/367)
- usb power source: [Anker PowerCore 26800](https://www.amazon.com/dp/B01JIWQPMW)
- shoulder straps (suspenders): [etsy](https://www.etsy.com/nz/listing/456446760/handmade-usa-blackbrowntan-leather-clip)
- waist strap (belt): [etsy](https://www.etsy.com/nz/listing/114576723/handmade-thick-leather-belt-mens-womens)

## user interface design

controller has a current mode and current param.

the current mode is being rendered with all the params for that mode.

here are the available ways to interface with the controller:

- press 1st button for previous mode
- press 2nd button for next mode
- press 3rd button for previous param
- press 4th button for next param
- turn the rotary encoder to change the current param
- hold 1st button until white then turn rotary encoder to change brightness
- hold 2nd button until white then turn rotary encoder to change color temperature

## circuit design

TODO

## textile design

TODO

## resources

- [Adafruit ESP32 learning resources](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather)
  - [ESP32 pinouts](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts)
  - [ESP32 guide to installing the Arduino IDE](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/using-with-arduino-ide).
- [arduino-esp32/.../examples/.../RepeatTimer.ino](https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino)

