# EDLAVP-ESP-FW

The EDLAVP-ESP-FW project is the ESP-IDF version of the EDLAVP firmware, designed to run on ESP32 modules.

If you do not know what EDLAVP is, you probably don't need to be here. I'll release more documentation and info on
EDLAVP at some point.

## Compatibility

Being an ESP-IDF project, you (or anyone) can "tune" this project to work with any ESP32 MCU.
For development, I'm using an **ESP32-WROOM-32D** on an official Espressif DevKit-C1 development board.

You should be able to build this project for any ESP32 chip, but you may need to make some changes.

## Hardware

For now, the only hardware this project uses/needs is a **DS18B20** temperature sensor probe (which requires a 4.7K
pullup resistor between VCC and the data line), and a couple of LEDs (with their current limiting resistors, obviously).

## PCB

I am planning to eventually make a PCB for this project, intended to have an ESP32 module soldered on it. This project
is still
in very early stages, so until I've gotten the code far enough for the project to be functional
on a breadboard, I will make the first prototype PCB. I will create a new repository with the PCB files (KiCad) once
that's done.

## Contributing

I'm not expecting anyone to contribute to this project, I'm just making this repo public in case anyone wants to play
with it.
If you do wish to contribute, you are very welcome to do so. You should create an issue for bugs or suggestions, and if
you want to
work on that issue yourself, feel free to fork and open a PR referencing the issue.

Note that if you fork this repository or create a new project out of it, you have to license it under GPLv3 too.
See [LICENSE](LICENSE) for more.

## License

EDLAVP-ESP-FW is licensed under the GNU General Public License v3 (GPLv3).  
See the [LICENSE](./LICENSE) file for details.