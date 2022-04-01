/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
package com.microej;

public class ImmutableArray implements ArrayPrinter {

	@Override
	public void startArray(int size) {
		System.out.println("<immutables>");
		System.out.println("\t<array id=\"circularArea\" type=\"int[]\">");
	}

	@Override
	public void startLine() {
		System.out.print("\t\t");
	}

	@Override
	public void addElement(int element) {
		System.out.print("<elem value=\"" + element + "\"/>");
	}

	@Override
	public void stopLine() {
		System.out.println();
	}

	@Override
	public void stopArray() {
		System.out.println("\t</array>");
		System.out.println("</immutables>");
	}

}
