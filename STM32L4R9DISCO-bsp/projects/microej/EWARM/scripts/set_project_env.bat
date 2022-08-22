@echo off

REM Copyright 2019-2022 MicroEJ Corp. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be found with this software.

REM 'set_project_env.bat' implementation for IAR Embedded Workbench.

REM 'set_project_env' is responsible for
REM - checking the availability of required environment variables 
REM - setting project local variables for 'build.bat' and 'run.bat' 

REM IAR Embedded Workbench installation directory (e.g.: C:\Program Files\IAR Systems\Embedded Workbench VERSION)
SET IAREW_INSTALLATION_DIR=C:\Program Files\IAR Systems\Embedded Workbench 9.0
REM IAREW project directory that contains the project file .ewp (e.g.: %~dp0\..\)
SET IAREW_PROJECT_DIR=%~dp0%\..
REM IAREW project file name without the extension .ewp (e.g.: Project)
SET IAREW_PROJECT_NAME=Project
REM IAREW project configuration (e.g.: Debug or Release)
SET IAREW_PROJECT_CONFIGURATION=Release

SET CUBE_PROGRAMMER_DIR=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin

SET IAREW_PROJECT_EXECUTABLE_FILE="%IAREW_PROJECT_DIR%\STM32L4R9I-Discovery\Exe\%IAREW_PROJECT_NAME%.out"

ECHO IAREW_INSTALLATION_DIR=%IAREW_INSTALLATION_DIR%
ECHO IAREW_PROJECT_DIR=%IAREW_PROJECT_DIR%
ECHO IAREW_PROJECT_NAME=%IAREW_PROJECT_NAME%
ECHO IAREW_PROJECT_CONFIGURATION=%IAREW_PROJECT_CONFIGURATION%
ECHO CUBE_PROGRAMMER_DIR=%CUBE_PROGRAMMER_DIR%
exit /B 0