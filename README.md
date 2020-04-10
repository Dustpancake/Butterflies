# Butterflies
Arduino and Python kinematic sculpture and music visualiser, written for an atmelavr uno MC.

## Setup
The whole project can be built using PlatformIO, with 
```bash
pio run
```
or uploaded to a connected MC
```bash
pio run -t upload
```

## Use and design
This sketch is designed to be used in conjunction with a Python script for music visualization. The basic operation of the script is to read in `N` bytes of data, where `N` is the number of LEDs and Servos connected (configurable in the head of `src/main.cpp`).

- Bytes for the LEDs are used to control the LED brightness from `0` to `255`.
- Bytes for the Servos are used to change the speed of oscillation between some `MIN_POS` and `MAX_POS`, calculated as
```cpp
speed = (log((float) data));
```
where `data` is the input byte.

The input bytes should be carriage return terminated `\r`, since the script flushes the buffer after each read, so that playback speed is better synchronized with the visualization.
