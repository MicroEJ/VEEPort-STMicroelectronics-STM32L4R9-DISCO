<?xml version="1.0"?>
<!--
	Front Panel
	
	Copyright 2019 MicroEJ Corp. All rights reserved.
	For demonstration purpose only.
	MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
-->
<frontpanel 
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="https://developer.microej.com" 
	xsi:schemaLocation="https://developer.microej.com .widget.xsd">
	
	<device name="STM32L4R9-DISCO" skin="back.png">
		<ej.fp.widget.Display x="11" y="222" width="390" height="390" filter="mask.png"/>
		<ej.fp.widget.Pointer x="11" y="222" width="390" height="390" filter="mask.png" touch="true"/>
		<ej.fp.widget.Joystick x="506" y="673" 
			upSkin="joystick_up.png" 
			downSkin="joystick_down.png" 
			rightSkin="joystick_right.png" 
			leftSkin="joystick_left.png"
			enterSkin="joystick_push.png" 
			skin="joystick_skin.png"
			filter="joystick_mask.png"
		/>
	</device>
</frontpanel>