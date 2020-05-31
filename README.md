# RBBA
resuscitator-based breathing assistance

This project explores the idea of building a breathing assistance system based on a resuscitator, which is safe enough for actual usage. This includes the use of sensors to limit the pressure supplied to a recipient and a calibration procedure that allows the system to estimate the volume of air given. In addition, the device should be printable on affordable, low-end 3D printers and use only hardware parts that are widely available. Likewise, the electronics involved should be as minimal as possible, easy to program, and adaptable to one's needs.

## Current Status

There exists a hardware prototype including a control-box that houses all the electronics. Furthermore, all basic software components are implemented and ready to be used for the user-end part of the code (which is still incomplete).

![device_photo](https://github.com/jkerdels/RBBA/blob/master/rbba-dev.jpg?raw=true "device prototype")
![controlbox_photo](https://github.com/jkerdels/RBBA/blob/master/rbba-devbox.jpg?raw=true "control box prototype")


## Geometry

The geometry of the hardware was modeled in OpenSCAD. This way, it is relatively easy to adapt the geometry, e.g., to work well with a particular 3D-printer. In addition, using OpenSCAD allows to automate the generation of all stl-files via a simple command line script.

![parametric_geometry](https://github.com/jkerdels/RBBA/blob/master/rbba-openscad.jpg?raw=true "device geometry")

## Electronics & Code

The electronics are based on the Arduino platform in order to make the code and hardware as accessible as possible. The arduino project does not rely on external libraries and brings everything with it to, e.g., read out sensors or write messages to the output display. The main board used is an Arduino Nano compatible device. Additional modules include the BMP280 pressure sensor, a L298N motor driver, a HD44780 display with an I2C-Driver (FC-113), and a rotary encoder input dial.

