# Homie

## Boblight Release

Control WS2812 LEDs from a  ESP8266 (nodemcu)

## Development
* File
 * Open Workspace
  * Select the file in this repository:  HomieBob.code-workspace

## Filesystem
### Configuration
Use the config-example.json from the host folder and create here a config.json file.
### HowTo upload
Start Platform.io
Open a new Atom-Terminal and generate the filesystem with the following command :
```pio run -t buildfs```
Upload this new generated filesystem with:
```pio run -t uploadfs```

# Hardware
* D4: WS2812 RGB LEDs

### Hardware

* PC / Raspberry to run boblight e.g. in combination with [Kodi](kodi.tv)
* WS2812 LEDs
* ESP8266 (e.g. nodemcu)
  * Hardware interface
    * GPIO2 is connected with the data input of the first WS2812
    * GND is also connected with the WS2812
    * 5V power supply, can be the same as for the WS2812
    * USB-Connecten for the USB serial

### Software

* [Boblight](https://code.google.com/p/boblight/)
* This Project
* Configuration, see below

### Configuration
The **boblight.conf** used to communicate with the STM should look like the following:
```
[global]
#interface      10.0.0.2
#port           19333

[device]
name            ambilight1
output          /dev/ttyACM0
channels        30
type            momo
prefix          41 64 61 00 28 7D
interval        20000
rate            115200
#debug           on
#allowsync       yes

[color]
name            red
rgb             FF0000
gamma           2.1
adjust          0.12
blacklevel      0.01

[color]
name            green
rgb             00FF00
gamma           2.1
adjust          0.12
blacklevel      0.01

[color]
name            blue
rgb             0000FF
gamma           2.1
adjust          0.12
blacklevel      0.01

[light]
name            main
color           red     ambilight1 1
color           green   ambilight1 2
color           blue    ambilight1 3
hscan   0 100
vscan   0 100
```

### Sources / Inspirations
* [STM32F4 Discovery board](https://github.com/0110/STMboblight)
* [Adalight](https://github.com/adafruit/Adalight/blob/master/Arduino/LEDstream_LPD8806/LEDstream_LPD8806.pde)

