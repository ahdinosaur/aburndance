# esp32-feather-pixels

## hardware

[Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)

## resources

- [Adafruit learning resources](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather)
- [arduino-esp32/.../examples/.../RepeatTimer.ino](https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino)

## how to get started

first, follow the [Adafruit guide to installing the Arduino IDE for this board](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/using-with-arduino-ide).

TODO...

## physical controller

- general brightness
- microphone sensitivity
- mode
  - x-way switch?
  - up and down buttons?
  - single next button?
  - one mode is "auto-cycle modes" (how long per mode)

## interfaces

- charge value
- multi-core
  - how to have one dedicated led message core
  - how to have other core for audio, wifi, buttons, etc
- microphones
  - how to i2s: https://github.com/espressif/arduino-esp32/issues/302
- apa102c leds
  - how to send messages to leds
    - gpio: https://github.com/pololu/apa102-arduino
    - spi: use spi pinouts with SPI interface: https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/pinouts#i2c-and-spi-pins
- wireless
  - how to connect to a wifi network
  - how to create a mesh (bluetooth)
- buttons and physical interfaces

## use cases

- pixel jockey
  - connect to wifi network
  - start tcp server to receive Open Pixel Control messages
  - broadcast Bonjour service with tcp server details
- collective sensing
  - if one node has a microphone sensor, they all "hear" each other
- listening
  - laughing mode
  - volume based on human noise level
- shapes
  - pillar
    - easy to daisy chain
  - hexa / octa yurt / teepee
    - controller at tip top
    - strips going down each side

## power

what is the power usage?
