# pwalkerHax

A port of Dmitry's work on [Pokewalker hacking](https://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker) as an homebrew application for the Nintendo 3DS.

It is essentially an hacking tool for the Pokewalker that uses the 3DS's infrared transceiver to communicate with the device.

## Current Features

At the moment, the following features are implemented:
- **Adding watts**
- **Gifting an event item**
- **Gifting an event Pokemon**

Feel free to open an issue if you have any suggestions or requests for new features.

When connecting to the Pokewalker, make sure to point it towards the 3DS's infrared sensor, which is the black spot on the back side of the device, and keep the two devices close to each other.

## Installation

Just download the latest [release](https://github.com/francesco265/pwalkerHax/releases) as a `.3dsx` file and execute it using the homebrew launcher.

## How to build

The only requirement to build this project is to have the [libctru](https://github.com/devkitPro/libctru) library installed on your system, then just run the `make` command in the root directory of the project.

## Credits

This code is just a port of Dmitry's amazing work, for which he originally developed a running demo for PalmOS devices.

TODO
