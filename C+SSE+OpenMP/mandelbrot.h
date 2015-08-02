/*   Copyright (C) 2013 Daniel Thürck
 
 *   This program is free software; you can redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.

 *   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License along with this program;
 *   if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 */
#ifndef MANDELBROT_HEADER
#define MANDELBROT_HEADER

#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "globals.h"

/*
 * Generates an image of a Mandelbrot set in the given bounded complex plane
 * for a given sampling specified in width times height pixels. Uses colormapping
 * to determine the pixel's color from its iteration return value.
 * 
 * Arguments:
 *	upperLeft - Upper left corner of the complex plane for calculation
 *	lowerRight - Lower right corner of the complex plane for calculation
 *	maxIterations - Maximum number of iterations that are executed to determine a series' boundedness
 *	width - Width of the resulting image in pixels (also defines horizontal sampling density)
 *	height - Height of the resulting image in pixels (also defines the vertical sampling density)
 *
 * Returns:
 *   An array of RGB 8-bit values (unsigned char) representing the color-mapped
 *   Mandelbrot set. Pixels are saved in row-major order.    
 */
unsigned char *
generateMandelbrot(
    complex float upperLeft,
    complex float lowerRight, 
    int maxIterations, 
    int width, 
    int height);

#endif /* MANDELBROT_HEADER */
