# aburndance

a portable rainbow

## how to get started

### simulator

[install rust](https://doc.rust-lang.org/book/second-edition/ch01-01-installation.html)

Debian Stretch dependencies

```shell
sudo apt install libgtk-3-dev
```

```shell
cargo run
```

## hardware

- [pocketbeagle](https://github.com/beagleboard/pocketbeagle)
- 1x rotary encoder (eQEP)
  - with reset button
- param selector buttons (+ and -)
- mode selector buttons (+ and -)
- brightness potentiometer
- apa102 spi out
- led power protection (like [AllPixel Power Tap Kit](https://www.seeedstudio.com/AllPixel-Power-Tap-Kit-p-2380.html))

nice to have:

- microphone sensitivity potentiometer
- microphone in
- audio jack in

## interfaces

- apa102 spi interface (like [fastled](https://github.com/FastLED/FastLED))
- params interface
- graphics (glsl) interface
  - input
    - params (param name, encoder value)
    - pixel position (x, y)
  - output: pixel color

nice to have

- audio i2s interface 

## resources

TODO
