# Description

This platform includes an accelerated algorithm to draw an image with rotation. 

# Limitations

This algorithm has some limitations:
* the source and destination images must have the same size (in pixels)
* the image bounds is a square
* the source and destination images must have the format `RGB565`
* source image cannot be transparent
* no mirror can be applied
* no global alpha can be applied 
* only a circular zone is rotated (see later)
* nearest neighbor method is used
* on sim, the standard algorithm `ImageRotation.drawNearestNeighbor()` is called instead (so the rendering is not exactly the same)

# Circular Area

## Description

The algorithm requires an integer array which defines the circular area to rotate. For each source line, 4 integers are required (x0 to x3):
* 0 to x0: area to drop (no rotation)
* x0 to x1: area to rotate
* x1 to x2: area to drop (no rotation)
* x2 to x3: area to rotate
* x3 to (w-1): area to drop (no rotation)

Note: x2 and x3 are equal to x1 when there is only one horizontal line to rotate for a given Y. 

## How to Generate

1- Using GIMP (or other), remove useless areas (external area and internal area): set these areas fully transparent.
2- Save this PNG image here: `${project_loc:/RotationExtractor/src/com/microej}`
3- Open `CircularRotationExtractor` and fill the image path
4- Run it
5- Copy result in an immutable file, example:
```
	<immutables>
		<array id="circularArea" type="int[]">
			<elem value="176"/><elem value="214"/><elem value="214"/><elem value="214"/>
			<elem value="167"/><elem value="223"/><elem value="223"/><elem value="223"/>
			<elem value="161"/><elem value="229"/><elem value="229"/><elem value="229"/>
			<...>
		</array>
	</immutables>
```
Note: the generator creates the immutable array `circularArea` but this name can be updated 

## How to Use

1- Paste `Rotate.java` in your application (package `com.microej.microui`)
2- Add an immutable list file (example `images.immutables.list` with the line `/resources/immutables/image390.data`)
3- Retrieve the immutable array in application: `int[] circularArea = (int[]) Immutables.get("circularArea");`
4- Call `Rotate.drawNearestNeighborCircularImage(g, model.getImage(), circularArea, angle);` 

Note: several images and circular area arrays can be used in same application.

# Misc

* The time to draw the rotated image must be found before fixing the application animation timer. This animation time must include the rotation time, the other algorithms, the flush time.
* For a given image and a given timer animation, the rendering can appear very differently depending on the step between two angles. When the step is small, the drawing appears slow but smooth. When the step is bigger, the drawing appears (very) fast but not smooth.


---
_Copyright 2019 MicroEJ Corp. All rights reserved._  
_MicroEJ Corp. PROPRIETARY. Use is subject to license terms._
