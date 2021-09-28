//	JLink Commands
//	
//	Copyright 2019-2021 MicroEJ Corp. All rights reserved.
//	For demonstration purpose only.
//	MicroEJ Corp. PROPRIETARY. Use is subject to license terms.

Device STM32L4R9AI
r
loadbin seggerJLink.bin,0x08000000
loadbin seggerJLinkExt.bin,0x90000000
h
r
g
qc