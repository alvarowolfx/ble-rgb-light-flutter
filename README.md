# BLE RGB Light and Flutter

End goal of this project is to control an RGB LED Light over BLE using a Flutter App.

This is a Work in Progress.

## Technologies used here

* Firmware is Based on Zephyr RTOS
  * Developed using PlatformIO with Zephyr support
* Flutter
* NRF52840 Dongle
  * [More details](https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52840-Dongle)
  * An ESP32 version will be pushed later. It's going to be easier, but I wanted to learn to work with a newer board/environment.

The board will present an Nordic UART Service over BLE, that is basically an serial communication over BLE. The board is going to accept the command in json format and reply back the state in json format.

Spec for commands:

```
{
  color : "FFFFFF"
}
```

Spec for state:

```
{
  color : "FFFFFF",
  r : 255,
  g : 255,
  b : 255
}
```

## Required Materials

* Only the NRF52840 Dongle
  * The board already have a built-in RGB LED

## Installing the firmware

* Need nrfutil installed - [Instructions](https://github.com/NordicSemiconductor/pc-nrfutil)
* Open the firmware folder on PlatformIO and run the `Build` command.
  * After the build command, the hex file will be generated and it's required for the next step.
* Put the board into bootloader mode (Press the reset button and the board will start to blink red slowly).
* Find your board serial port.
  * Run `ls /dev/tty.*`. Mine for example was `/dev/tty.usbmodemF4DF6C998E5A1`
* Run the `flash.sh` script with your port as a parameteer, that basically uses `nrfutil` to pack and write the firmware.
```
./flash.sh /dev/tty.usbmodemF4DF6C998E5A1
```

## Flutter App

App code is on the `ble_rgb_lamp`. The code right is super ugly and hacky, I'll separate it in more files to make it more understandable.

#### References

* A lot of the base firmware was based on this project:
  * https://github.com/larsgk/web-nrf52-dongle
* Similar project, but with different libraries and firmware :
  * https://medium.com/@pietrowicz.eric/bluetooth-low-energy-development-with-flutter-and-circuitpython-c7a25eafd3cf