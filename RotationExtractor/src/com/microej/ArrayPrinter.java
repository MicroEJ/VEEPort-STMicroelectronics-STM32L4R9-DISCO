/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
package com.microej;

public interface ArrayPrinter {

	void startArray(int size);

	void startLine();

	void addElement(int element);

	void stopLine();

	void stopArray();

}
