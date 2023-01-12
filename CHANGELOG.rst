..
	Copyright 2019-2023 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.

===========
 Changelog
===========

----------------------
 [1.0.5] - 2023-01-11
----------------------

Changed
=======

- Update the Debugging section of the Readme

----------------------
 [1.0.4] - 2022-08-26
----------------------

Changed
=======

- Use the DMAD2D 2.1.0 that improves performances when drawing an image.

Fixed
=====

- Fix the disabling of GFXMMU (fix the LCD frame buffer address).


----------------------
 [1.0.3] - 2022-08-19
----------------------

Changed
=======

- Update IAR version from 8.50.6 to 9.20.2.
- Update IAR project configuration (enable flash loader).

----------------------
 [1.0.2] - 2022-07-22
----------------------

Fixed
=====

- Fixed a version mismatch.

----------------------
 [1.0.1] - 2022-07-06
----------------------

Added
=====

- Verify flash operation in ``run.bat`` script.
- bsp readme.

Changed
=======

- Update readme.
- Update release note.


Fixed
=====

- Run software after flashing in run.bat.

----------------------
 [1.0.0] - 2022-04-01
----------------------

Added
=====

- Update architecture to 7.16.0.
- Update UI pack to 13.1.0.
- Add device pack 1.1.1.
- Add Platform Configuration Additions 1.4.0.
- Add build and run scripts.
- Add helloworld application files to compile the BSP without any platform.
- Add release notes.

Changed
=======

- Update UI support for UI pack 13.x.
- Refactor project to be consistent with Platform template.
- Remove framerate task.
- Update readme.
- Update copyrights and license.

Removed
=======

- Remove GIML pack.
- Remove deploy tools.

Fixed
=====

- Improve interrupts support using FreeRTOS's ``xPortIsInsideInterrupt()``.
- Increase joystick repeat period to 500ms.

----------------------
 [0.1.0] - 2021-09-28
----------------------

Added
=====

- Add architecture 7.11.0.
- Add UI pack 12.0.2.
- ADD GIML pack 3.1.0.
- Add core support.
- Add UI support (display, touch, joystick).
- Add trace support (SystemView).
- Add custom support to draw images with rotation efficiently.
- Initial release of the platform.
