# pwalkerHax

An hacking tool for the Pokewalker written as a 3DS homebrew application that uses the built-in infrared transceiver to communicate with the Pokewalker.

You might also be interested in my other project [**RtcPwalker**](https://github.com/francesco265/RtcPwalker).
[Here](https://youtu.be/f6f8RSxqG20) you can find a demo of my two projects in action (**pwalkerHax** + **RtcPwalker**).


## Current Features

At the moment, the following features are implemented or planned:
- [x] **Add watts**
- [x] **Set today steps**
- [x] **Set total steps to 9.999.999**
- [x] **Gift an event item**
- [x] **Gift an event Pokemon**
- [x] **Dump ROM**
- [x] **Dump EEPROM**


Feel free to open an issue if you have any suggestions or requests for new features.

When connecting to the Pokewalker, make sure to point it towards the 3DS's infrared sensor, which is the black spot on the back side of the device, and keep the two devices close to each other.

### Commands

| Command    | Action                |
|------------|-----------------------|
| Up/Down    | Move selection        |
| Left/Right | Move selection faster |
| A          | Select                |
| B          | Cancel                |
| Y          | Go to item number     |

## Installation

Just download the latest [release](https://github.com/francesco265/pwalkerHax/releases) as a `.3dsx` file and execute it using the homebrew launcher.

## How to build

The only requirement to build this project is to have the [libctru](https://github.com/devkitPro/libctru) library installed on your system, then just run the `make` command in the root directory of the project.

## Credits

This code is just a port of Dmitry's amazing [work](https://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker), for which he originally developed a running demo for PalmOS devices. He reverse-engineered the Pokewalker's protocol and created all the exploits, i just ported his work to the 3DS.

---
If you appreciate my works and want to support me, you can offer me a coffee :coffee::heart:.

- **BTC**: `bc1qvvjndu7mqe9l2ze4sm0eqzkq839m0w6ldms8ex`
- **PayPal**: [![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=AAZDH3SM7T9P6)
