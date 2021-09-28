# MicroEJ STM32L4R9-DISCO Platform

This project is used to build MicroEJ platform for STM32L4R9-DISCO as well as its reference implementation.

## Supported board

* STM32L4R9-DISCO (MB1311C) with LCD extension board MB1314

## Hardware Features

* CPU frequency: up to 120MHz
* Internal flash: 2MB
* Internal RAM: 640KB (192+64+384 where the 64KB are "hardware parity checked")
* External PSRAM: 2MB
* Printf / println on USART 2 (USB)

## Benchmarks

### RAM speed (2019-07-01)
* RAM speed average read access (according to your configuration file 8/16/32 bits) : 8.583069MBytes/s 
* RAM speed average write access (according to your configuration file 8/16/32 bits) : 13.987223 MBytes/s 
* RAM speed average transfert access (according to your configuration file 8/16/32 bits) : 13.987223MBytes/s 

### PSRAM

## MicroEJ Features

* Sim platform only (Emb platform not ready yet)
* Basic front panel with display 390x390 16 BPP (RGB565) + touch

## Notes

* no KF, no FS, no ECOM

---
_Copyright 2019 MicroEJ Corp. All rights reserved._  
_For demonstration purpose only._  
_MicroEJ Corp. PROPRIETARY. Use is subject to license terms._
