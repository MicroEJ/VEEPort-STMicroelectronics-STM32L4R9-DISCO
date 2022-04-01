<?xml version="1.0"?>
<!--
	Copyright 2019-2022 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.
-->
<frontpanel 
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="https://developer.microej.com" 
	xsi:schemaLocation="https://developer.microej.com .widget.xsd">
	
	<device name="STM32L4R9-DISCO" skin="back_small.png">
		<ej.fp.widget.Display x="11" y="51" width="390" height="390" filter="mask.png"/>
		<ej.fp.widget.Pointer x="11" y="51" width="390" height="390" filter="mask.png" touch="true"/>
		<ej.fp.widget.Joystick x="400" y="320" 
			upSkin="joystick_up.png" 
			downSkin="joystick_down.png" 
			rightSkin="joystick_right.png" 
			leftSkin="joystick_left.png"
			enterSkin="joystick_push.png" 
			skin="joystick_skin.png"
			filter="joystick_mask.png"
			repeatPeriod="500"
		/>
	</device>
</frontpanel>