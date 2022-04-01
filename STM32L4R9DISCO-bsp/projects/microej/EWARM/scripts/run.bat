@echo off

REM Copyright 2020-2022 MicroEJ Corp. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be found with this software.

REM 'run.bat' implementation for STM32CubeIDE.

REM 'run.bat' is responsible for flashing the executable file on the target device 
REM then resetting target device

CALL "%~dp0\set_project_env.bat"
IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

IF "%~1"=="" (
	SET APPLICATION_FILE=%IAREW_PROJECT_EXECUTABLE_FILE%
) ELSE (
	SET APPLICATION_FILE=%~f1
)

IF NOT EXIST "%APPLICATION_FILE%" (
	echo FAILED - file '%APPLICATION_FILE%' does not exist
	exit /B 1
)

REM Save application current directory
SET CURRENT_DIRECTORY=%CD%

REM Jump this script's directory
CD %~dp0%

@echo on

"%CUBE_PROGRAMMER_DIR%\STM32_Programmer_CLI.exe" -c port=SWD mode=UR -w "%APPLICATION_FILE%" -el "%CUBE_PROGRAMMER_DIR%\ExternalLoader\MX25LM51245G_STM32L4R9I-DISCO.stldr" -rst

CD "%CURRENT_DIRECTORY%"
