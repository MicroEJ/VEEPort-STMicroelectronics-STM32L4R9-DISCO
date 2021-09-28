/*
 * Java
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * MicroEJ Corp. PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.microej.microui;

import ej.bon.Constants;
import ej.microui.display.GraphicsContext;
import ej.microui.display.Image;
import ej.microui.display.Image.OutputFormat;
import ej.microui.display.transform.ImageRotation;

/**
 * This class allows to draw an image with a rotation using an accelerated
 * algorithm. This algorithm is optimized when the following characteristics are
 * respected (otherwise an {@link IllegalArgumentException} is thrown).
 *
 * <li>The image to draw has exactly the same size than the destination</li>
 * <li>The image is a square</li>
 * <li>The image format is {@link OutputFormat#RGB565}</li>
 * <li>The display format is {@link OutputFormat#RGB565}</li>
 * <li>No mirror can be applied</li>
 * <li>No global transparenct (alpha) can be applied</li>
 * <li>Only a circular area is drawn (not the full image)</li>
 */
public class Rotate {

	private static final String MICROUI_PROPERTY_RUN_ON_S3 = "com.microej.library.microui.onS3";

	private Rotate() {
		// private constructor: this class cannot be instanciated
	}

	/**
	 * Rotates an image using nearest neighbor algorithm. This algorithm is faster
	 * than <code>bilinear</code> algorithm but its rendering is more simple.
	 * <p>
	 * Rotates only a subpart of image. This sub part is defined by the given
	 * circular area. This area has been previously extracted from image to render
	 * using the external tool <code>CircularRotationExtractor</code>.
	 *
	 * @param gc
	 *            the graphics context where draw the rotated image.
	 * @param image
	 *            the image to render
	 * @param circularArea
	 *            the circular area to rotate
	 * @param angle
	 *            the rotation angle
	 */
	public static void drawNearestNeighborCircularImage(GraphicsContext gc, Image image, int[] circularArea,
			int angle) {
		if (Constants.getBoolean(MICROUI_PROPERTY_RUN_ON_S3)) {
			drawOnSimulator(gc, image, angle);
		} else {
			if (circularArea.length != image.getHeight() * 4) {
				// circular area must contain 4 elements for each line
				throw new IllegalArgumentException();
			}
			checkResult(drawCircularImageWithArray(gc.hashCode(), image.hashCode(), circularArea, angle));
		}
	}

	/**
	 * Draw image with rotation on the simulator. The "circular" mode is not
	 * supported.
	 *
	 * @param gc
	 *            the graphics context where draw the rotated image.
	 * @param image
	 *            the image to render
	 * @param angle
	 *            the rotation angle
	 */
	private static void drawOnSimulator(GraphicsContext gc, Image image, int angle) {
		ImageRotation r = new ImageRotation();
		r.setRotationCenter(image.getWidth() / 2, image.getHeight() / 2);
		r.setAngle(angle);
		r.drawNearestNeighbor(gc, image, image.getWidth() / 2, image.getHeight() / 2,
				GraphicsContext.HCENTER | GraphicsContext.VCENTER);
	}

	private static void checkResult(int result) {
		if (result != 0) {
			// image and/or graphics context are not in RGB888 format
			throw new IllegalArgumentException();
		}
	}

	private static native int drawCircularImageWithArray(int gcID, int imageID, int[] circularArea, int angle);
}
