..
    Copyright 2022 MicroEJ Corp. All rights reserved.
    Use of this source code is governed by a BSD-style license that can be found with this software..

.. |BOARD_NAME| replace:: STM32L4R9I-DISCO
.. |PLATFORM_VER| replace:: 1.0.2
.. |PLATFORM| replace:: MicroEJ Platform
.. |MANUFACTURER| replace:: STMicroelectronics

========================================================
|PLATFORM| Release Notes for |MANUFACTURER| |BOARD_NAME|
========================================================

Description
===========

This is the release notes of the |PLATFORM| for |BOARD_NAME|.

Versions
========

Platform
--------

|PLATFORM_VER|

Dependencies
------------

This |PLATFORM| contains the following dependencies:

- MicroEJ IAR specific architecture and packs:

.. list-table::
   :header-rows: 1
   
   * - Dependency Name
     - Version
   * - flopi4I35 (Architecture)
     - 7.16.0
   * - flopi4I35-ui-pack
     - 13.1.0

- MicroEJ generic packs:

.. list-table::
   :header-rows: 1
   
   * - Dependency Name
     - Version
   * - device-pack
     - 1.1.1

Please refer to the |PLATFORM| `module description file <./STM32L4R9DISCO-configuration/module.ivy>`_ 
for more details.

Board Support Package
---------------------

- BSP provider: |MANUFACTURER|
- BSP version: v1.14.0

Please refer to the |MANUFACTURER| GitHub git repository
available `here
<https://github.com/STMicroelectronics/STM32CubeL4>`__.

Third Party Software
--------------------

Third party softwares used in BSP are included in the |MANUFACTURER|
GitHub git repository available `here <https://github.com/STMicroelectronics/STM32CubeL4>`__.
Here is a list of the most important ones:

.. list-table::
   :header-rows: 1
   :widths: 3 3 3

   * - Software
     - Version
   * - FreeRTOS
     - 10.0.1

Features
========

Graphical User Interface
------------------------

This |PLATFORM| features a graphical user interface. It includes a display,
a touch panel and a joystick.

Display
~~~~~~

This |PLATFORM| features a 390 * 390 AMOLED round display.  The pixel format
is 16 bits-per-pixel (RGB565).  The display device is connected to the MCU
via a MIPI DSI.

The MicroUI back buffer is located in RAM and the MicroUI images heap is located in external RAM.

Known issues/limitations
========================

- Enable to reset the software after a flash
    - **Brief**: After a flash program and reset operation from the debug link, the software is not able to start correctly and displays a black screen. A manuel cold reset is required.
    - **Symptoms** : A black screen with a PC at 0x1FFF 0000 - 0x1FFF 7000 (System memory) range instead at 0x0800 0000-0x0820 0000 (Flash memory) range.
    - **Workaround** : Unplug/replug the STM32L4R9DISCO power supply.


Platform Memory Layout
======================

Memory Sections
---------------

Each memory section is discribed in the IAR linker file available
`here
<STM32L4R9DISCO-bsp/projects/microej/EWARM/stm32l4r9xx_flash.icf>`__.

Memory Layout
-------------

.. list-table::
   :header-rows: 1
   
   * - Section Description
     - Section Name
     - Memory Type
   * - Initialization vector
     - ``.intvec``
     - ROM
   * - C code
     - ``.text``
     - ROM
   * - C read-only data
     - ``.rodata``
     - ROM
   * - MicroEJ Application and Library code
     - ``.text.soar``
     - ROM
   * - MicroEJ Application resources 
     - ``.rodata.resources``
     - OSPI
   * - MicroEJ Application images 
     - ``.rodata.images``
     - OSPI
   * - C static
     - ``.bss``
     - RAM
   * - C stack
     - ``CSTACK``
     - RAM
   * - C heap
     - ``HEAP``
     - RAM
   * - MicroEJ Application static
     - ``.bss.soar``
     - RAM
   * - MicroEJ Application threads stack blocks
     - ``.bss.vm.stacks.java``
     - RAM
   * - MicroEJ Core Engine internal heap
     - ``ICETEA_HEAP``
     - RAM
   * - MicroEJ Application heap
     - ``_java_heap``
     - RAM
   * - MicroEJ Application Immortal Heap
     - ``_java_immortals``
     - RAM
   * - MicroUI back buffer
     - ``DISPLAY_MEM``
     - RAM
   * - MicroUI images heap
     - ``.bss.microui.display.imagesHeap``
     - SDRAM

Please also refer to the MicroEJ docs website page available `here
<https://docs.microej.com/en/latest/PlatformDeveloperGuide/coreEngine.html#link>`__
for more details.
