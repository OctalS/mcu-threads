mcu-threads
===========
Version: 1.0.0

Small and compact library to implement multitasking
in embedded systems where resources are scarce.
This lib can run even on a chip with less
than 1K of ram!

In order to use mcu-threads your chip needs to support:

* at least one hardware timer
* at least 512 bytes of ram (it's able to run with
        even less but it's not worth it.)

Supported devices
-----------------
The lib is designed to support different devices.
It has a built in interface to easily add support for
new devices. Please refer to arch/ directory for a list
of all supported devices.

Building
--------
To build invoke:

        make ARCH=<your arch> CPU=<cpu variant, this is optional>

This will produce a static library and a header in out/

Usage
-----
For details on using the library read doc/API

Adding support for other architectures and devices
--------------------------------------------------
Please refer to:
* doc/ARCH 
* doc/ARCH_API

Contact
-------
author: Vladislav Levenetz

email: octal.s@gmail.com

https://github.com/OctalS/mcu-threads.git
