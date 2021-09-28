/*
 * Java
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */
package com.microej;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

/**
 *
 */
public class CircularRotationExtractor {

	public static final String IMAGE_PATH = "E:\\work\\eclipse\\workspaces\\_P0171_STM32L4R9-DISCO\\RotationExtractor\\src\\com\\microej\\390.png";
	public static final int USELESS_COLOR = 0xffffff;
	public static final ArrayPrinter PRINTER = new ImmutableArray();

	public static void main(String[] args) {
		BufferedImage image = loadImage();
		int width = image.getWidth();
		int height = image.getHeight();

		PRINTER.startArray(height * 4);

		for (int y = 0; y < height; y++) {
			int cpt = 0;
			boolean lookForNonTransparentPixel = true;
			int lastX = 0;
			PRINTER.startLine();
			int x = 0;
			for (; x < width; x++) {
				int pixel = image.getRGB(x, y);
				if (lookForNonTransparentPixel) {
					if ((pixel >> 24) != 0) {
						PRINTER.addElement(x);
						lookForNonTransparentPixel = false;
						++cpt;
						lastX = x;
					}
				} else {
					if ((pixel >> 24) == 0) {
						PRINTER.addElement(x);
						lookForNonTransparentPixel = true;
						++cpt;
						lastX = x;
					}
				}
			}
			if (cpt == 3) {
				--x;
				PRINTER.addElement(x);
				++cpt;
				lastX = x;
			}
			for (int i = cpt; i < 4; i++) {
				PRINTER.addElement(lastX);
			}
			PRINTER.stopLine();
		}
		PRINTER.stopArray();
	}

	private static BufferedImage loadImage() {
		BufferedImage bi;
		try {
			bi = ImageIO.read(new File(IMAGE_PATH));

			if (bi == null) {
				// it is not an image!
				return null;
			}

			return bi; // valid file
		} catch (IOException e) {
			return null; // io error
		}
	}
}
