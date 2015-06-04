
nRF8001
=======

Last updated: 2015/06/04 

Provides simple API for nRF8001 BLE chip such as the [BLE Shield](http://redbearlab.com/bleshield/) and [Blend Micro](http://redbearlab.com/blendmicro/) to exchange data via RBL TxRx Service to BLE Central device (e.g. iPhone 5).

Other 3rd party breakout breads may also be workable.

Tested with Arduino IDE 1.6.4 (we will only support this version).

Installation
============

Step 1. You have to see this for installing libraries first.
http://www.arduino.cc/en/Guide/Libraries

* Please use the method - "Importing a .zip Library" at this moment.

Step 2: Get latest release of [Nordic nRF8001 SDK for Arduino](https://github.com/Cheong2K/ble-sdk-arduino/archive/RBL.zip).

Step 3: Get latest release of [RBL nRF8001 API](https://github.com/RedBearLab/nRF8001/archive/master.zip).

Step 4: Load an example to your Arduino board.


How to use
==========


The library structure and dependency:
YourSketch.ino -> RBL_nRF8001 -> Nordic's BLE

Also, you can use Nordic's library directly such as:
YourSketch.ino -> Nordic's BLE

Nordic also provides many examples and tutorials.

Read Nordic's BLE SDK for Arduino for details with tutorials to write your own services.

There are two Apps available from the Apple AppStore:<br/>

1. BLE Arduino<br>
It is for the BLEFirmata sketch and works for iOS 6

2. BLE Controller<br>
It is for the BLEController sketch and works for iOS 7


Change BLE Advertising Name
===========================

Before calling to ble_begin(), you can make use of ble_set_name("My BLE") to change the name.


Supported Boards
================

Arduino UNO (328p), Leonardo (32u4), MEGA2560, DUE and their compatible.<br/>
ChipKit Uno32<br/>


API
===

### ble_begin

```
void ble_begin();
```

* ble_begin starts the BLE stack and broadcasting the advertising packet 

### ble_set_name

```
void ble_set_name(char *name);
```

* Call ble_set_name by giving name before calling to ble_begin to set the broadcasting name. 

### ble_write

```
void ble_write(unsigned char data);
```

* ble_write sends a single byte data to the BLE Central.

### ble_write_bytes

```
void ble_write_bytes(unsigned char *data, unsigned char len);
```

* ble_write_bytes writes an array of bytes in data with length in len.

### ble_do_events

```
void ble_do_events();
```

* ble_do_events allows the BLE to process its events, if data is pending, it will be sent out.

### ble_read

```
int ble_read();
```

* ble_read reads a byte from BLE Central, It returns -1 if nothing to be read.

### ble_available

```
unsigned char ble_available();
```

* Returns the number of bytes ready for reading.

### ble_connected

```
unsigned char ble_connected(void);
```

* ble_connected returns 1 if connected by BLE Central or 0 if not.

### ble_set_pins

```
void ble_set_pins(uint8_t reqn, uint8_t rdyn);
```

* ble_set_pins is to specify the REQN and RDYN pins to the BLE chip, i.e. the jumper on the BLE Shield.

### ble_busy

```
unsigned char ble_busy();
```

* ble_busy return the status if the BLE is busy, i.e. it is using the SPI interface. It is for using the SPI for other components at the same time.

### ble_reset

```
void ble_reset(uint8_t reset_pin);
```

* ble_reset resets the BLE chip using the specified pin, reset_pin


Resources
=========

1. [Nordic nRF8001 SDK for Arduino - Library](https://github.com/Cheong2K/ble-sdk-arduino)

2. [Nordic nRF8001 SDK for Arduino - Forum](https://redbearlab.zendesk.com/forums/21921933-Nordic-nRF8001-SDK-for-Arduino)

3. [Nordic Developer Zone](https://devzone.nordicsemi.com/)

4. [Bluetooth SIG](https://www.bluetooth.org/en-us)


License
=======

Copyright (c) 2012-2014 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

