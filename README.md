# pwalkerHax

An hacking tool for the Pokewalker written as a 3DS homebrew application that uses the built-in infrared transceiver to communicate with the Pokewalker.

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

This code is just a port of Dmitry's amazing [work](https://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker), for which he originally developed a running demo for PalmOS devices. He reverse-engineered the Pokewalker's protocol and created all the exploits, i just ported his work to the 3DS.
