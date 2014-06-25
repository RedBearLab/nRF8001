nRF8001
=======

Provides API for nRF8001 BLE chip such as [BLE Shield](http://redbearlab.com/bleshield/) and [Blend Micro](http://redbearlab.com/blendmicro/).

Other 3rd party breakout breads may also be workable.


Installation
============

Step 1: Get latest release of Nordic nRF8001 SDK for Arduino here:<br/>
https://github.com/NordicSemiconductor/ble-sdk-arduino/releases

The library folder is "BLE" in this package.

Step 2: Get latest release of RBL nRF8001 API here:<br/>
https://github.com/RedBearLab/nRF8001/releases

The library folder is "RBL_nRF8001" in this package.

Step 3: Unzip these two packages, import or copy these two libraries, with folder named "BLE" and "RBL_nRF8001" to your Arduino sketchbook folder.

If you do not know how to do, read this:<br/>
http://arduino.cc/en/Guide/Libraries

Step 4: Load an example to your Arduino board.


How to use
==========


The library structure and dependency:
YourSketch.ino -> RBL_nRF8001 -> Nordic's BLE

Also, you can use Nordic's library directly such as:
YourSketch.ino -> Nordic's BLE

Nordic also provides many examples and tutorials.

Read Nordic's BLE SDK for Arduino for details with tutorials to write your own services, you can use this one directly:<br/>
https://github.com/NordicSemiconductor/ble-sdk-arduino

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


Resources
=========

1. [Nordic nRF8001 SDK for Arduino - Library](https://github.com/NordicSemiconductor/ble-sdk-arduino)

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

