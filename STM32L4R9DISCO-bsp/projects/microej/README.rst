.. 
    Copyright 2014-2023 MicroEJ Corp. All rights reserved.
    Use of this source code is governed by a BSD-style license that can be found with this software.

.. |BOARD_NAME| replace:: STM32L4R9-DISCO
.. |PLATFORM_VER| replace:: 1.0.5
.. |RCP| replace:: MICROEJ SDK
.. |PLATFORM| replace:: MicroEJ Platform
.. |PLATFORMS| replace:: MicroEJ Platforms
.. |SIM| replace:: MicroEJ Simulator
.. |ARCH| replace:: MicroEJ Architecture
.. |CIDE| replace:: MICROEJ SDK
.. |RTOS| replace:: FreeRTOS RTOS
.. |MANUFACTURER| replace:: STMicroelectronics

.. _README: ./../../../README.rst
.. _RELEASE NOTES: ./../../../RELEASE_NOTES.rst
.. _CHANGELOG: ./../../../CHANGELOG.rst

================
|BOARD_NAME| BSP
================

This project contains the BSP sources of the |PLATFORM| for the
|BOARD_NAME|.

This document does not describe how to setup the |PLATFORM|.  Please
refer to the `README`_ for that.

Build & Flash Scripts
---------------------

In the folder ``project/microej/EWARM/scripts`` 
for the IAR toolchain are scripts that can be used to build and flash the BSP.

- The ``build.bat`` scripts are used to compile and link the BSP with a
  MicroEJ Application to produce a MicroEJ Firmware
  (``Project.out``) that can be flashed on a device.

- The ``run.bat`` scripts are used to flash a MicroEJ Firmware
  (``Project.out``) on a device.

These scripts work out of the box, assuming the toolchain is
installed in the default path.

The following environment variables are customizable:  

**IAR toolchain**

- ``IAREW_INSTALLATION_DIR``: The path to IAR installation directory (already set to the default IAR Workbench default installation directory).
- ``IAREW_PROJECT_CONFIGURATION``: The project configuration (e.g. ``Release``).
- ``IAREW_PROJECT_DIR``: The directory that contains the ``Project.eww`` IAR project file (set to ``%~dp0``: the directory that contains the executed ``.bat``).
- ``IAREW_PROJECT_NAME``: The Eclipse CDT project name (``Project`` by default).

IAR Embedded Workbench IDE version 9.20.2 do not contains |BOARD_NAME| flashloader.
The ``run.bat`` script is using STM32CubeProgrammer to flash |BOARD_NAME| board.

- ``CUBE_PROGRAMMER_DIR``: The path to STM32CubeProgrammer installation directory (already set to the default STM32CubeProgrammer default installation directory).

The environment variables can be defined globally by the user or in
the ``set_local_env.bat`` scripts.  When the ``.bat`` scripts
are executed, the ``set_local_env.bat`` script is executed if it exists.
Configure these files to customize the environment locally.

Board Configuration
-------------------

|BOARD_NAME| provides several connectors, each connector is used by the MicroEJ Core Engine itself or by a foundation library.

Mandatory Connectors
~~~~~~~~~~~~~~~~~~~~

|BOARD_NAME| provides a multi function USB port used as:

- Power supply connector
- Probe connector
- Virtual COM port

Ensure the Power Supply jumper JP3 is fit to the left pin couple: 3.3V link (default setting).
Then just plug a mini USB Type-B cable from a computer to power on the board, to be able to program an application on it and see the traces.

For a detailed Power Supply setup check the user manual on ST website under `Resources <https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`__ tab.


Debugging with the |BOARD_NAME|
-------------------------------

Debugging with IAR 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Open the IAR project in IAR Embedded Workbench (open the file ``-bsp/projects/microej/EWARM/application.eww`` from IAR Embedded Workbench or by double-clicking on it from the MicroEJ SDK).
- Disable ``Enforce Memory Configuration`` option in the ST-Link menu. 
- Go to ``Project`` > ``Options...`` menu.
- Disable ``Verify Download`` property in the ``Debugger`` > ``Download`` section.
- Compile and link firmware by clicking on ``Project`` > ``Make`` button or Press 'F7'.
- Connect the |BOARD_NAME| to your computer.
- Flash your board using the ``run.bat`` script available in the ``project/microej/EWARM/scripts`` folder.
- Start the debug session by clicking on ``Project`` > ``Debug without Downloading``.
