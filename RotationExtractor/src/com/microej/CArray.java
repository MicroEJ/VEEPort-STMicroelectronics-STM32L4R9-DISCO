/*
 * Java
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */
package com.microej;

/**
 *
 */
public class CArray implements ArrayPrinter {

	@Override
	public void startArray(int size) {
		System.out.println("const uint32_t pixels[" + size + "] = ");
		System.out.println("{");
	}

	@Override
	public void startLine() {
		System.out.print("\t ");
	}

	@Override
	public void addElement(int element) {
		System.out.print(element + ", ");
	}

	@Override
	public void stopLine() {
		System.out.println();
	}

	@Override
	public void stopArray() {
		System.out.println("};");
	}

}
