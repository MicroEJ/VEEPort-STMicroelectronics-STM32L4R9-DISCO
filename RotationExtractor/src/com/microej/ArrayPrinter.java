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
public interface ArrayPrinter {

	void startArray(int size);

	void startLine();

	void addElement(int element);

	void stopLine();

	void stopArray();

}
