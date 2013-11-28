/*
 *   Copyright (C) 2013 Daniel Thürck
 *   Copyright (C) 2013 Stefan Schmidt
 
 *   This program is free software; you can redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.

 *   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License along with this program;
 *   if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 */
#include "mandelbrot.h"
#include "stdio.h"

/*
 * Addiert zwei komplexe Zahlen
 *
 * Arguments:
 *  a - erster Summand
 *  b - zweiter Summand
 * 
 * Returns:
 *  Die Summe der beiden komplexen Zahlen
 */
complex float complex_add (complex float a, complex float b)
{
	float x = crealf(a)+crealf(b);
	float y = cimagf(a)+cimagf(b);
	return x + y*I;
}

/*
 * Subtrahiert zwei komplexe Zahlen
 *
 * Arguments:
 *  a - Minuend
 *  b - Subtrahend
 * 
 * Returns:
 *  Die Differenz der beiden komplexen Zahlen
 */
complex float complex_sub (complex float a, complex float b)
{
	float x = crealf(a)-crealf(b);
	float y = cimagf(a)-cimagf(b);
	return x + y*I;
}

/*
 * Multipliziert zwei komplexe Zahlen
 *
 * Arguments:
 *  a - erster Faktor
 *  b - zweiter Faktor
 * 
 * Returns:
 *  Das Produkt der beiden komplexen Zahlen
 */
complex float complex_mul (complex float a, complex float b)
{
	float x = crealf(a)*crealf(b) - cimagf(a)*cimagf(b);
	float y = crealf(a)*cimagf(b) + cimagf(a)*crealf(b);
	return x + y*I;
}

/*
 * Berechnet den Betrag einer komplexen Zahl
 *
 * Arguments:
 *  c - komplexe Zahl
 * 
 * Returns:
 *  Der Betrag der komplexen Zahl
 */
float complex_abs (complex float c)
{
	return sqrt(pow(crealf(c),2.0f) + pow(cimagf(c),2.0f));
}

/*
 * Calculates a color mapping for a given iteration number by exploiting the
 * YUV color space. Returns the color as 8-bit unsigned char per channel (RGB).
 *
 * Arguments:
 *	index - Number of iterations that resulted from iterating the Mandelbrot series.
 *	maxIterations - Parameter that was also used for series iteration.
 *
 * Returns:
 *	The associated color as an array of 3 8-bit unsigned char values.
 */
void
colorMapYUV(int index, int maxIterations, unsigned char* color)
{
    // Wenn die komplexe Zahl Teil der Mandelbrotmenge ist, wird sie schwarz eingefärbt
	if (index == maxIterations) {
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		return;
	}
    
	// Farbschema anwenden
    float y,u,v;
    y = 0.2f;
    u = -1.0f + 2.0f * ((float)index / (float)maxIterations);
    v = 0.5f - ((float)index / (float)maxIterations);
	
	// YUV -> RGB
	float r,g,b;
	r = y + 1.28033f*v;
	g = y - 0.21482f*u - 0.38059f*v;
	b = y + 2.12798f*u;
	
	// RGB-Werte ins Array schreiben, dabei den Wertebereich von [0;1] auf [0;255] skalieren
	color[0] = (unsigned char) (r*255.0f);
	color[1] = (unsigned char) (g*255.0f);
	color[2] = (unsigned char) (b*255.0f);
}

/*
 * Executes the complex series for a given parameter c for up to maxIterations
 * and saves the last series component in last.
 *
 * Arguments:
 *  c - Additive component (complex number) for Mandelbrot series.
 *	maxIterations - Maximum number of iterations that are executed to determine a series' boundedness
 *	last - Pointer to a complex float number that can be used for storing the last component in a series - useful for color mapping
 *
 * Returns:
 *	The number of iterations that were executed before the series escaped our
 *	circle or - if the point is part of the Mandelbrot set a special
 *	(user-defined) value.
 */
int
testEscapeSeriesForPoint(complex float c, int maxIterations, complex float * last)
{
    complex float z = 0.0f + 0.0f*I;
	int iteration = 0;
	
	// Mandelbrotfolge durchgehen bis wir die Anzahl der erlaubten Iterationen erreicht haben oder der Betrag den Radius übersteigt
	while ((complex_abs(z) <= RADIUS) && (iteration < maxIterations))
	{
		z = complex_add(complex_mul(z,z), c);
		iteration++;
	}
	
	// Wenn die komplexe Zahl c nicht in der Mandelbrot-Menge liegt, smooth coloring anwenden
	if (iteration < maxIterations)
	{
		iteration += 1.0f - (log(log(complex_abs(z)) / log(2.0f)) / log(2.0f));
	}
	
	return iteration;
}

/*
 * Generates an image of a Mandelbrot set.
 */
unsigned char *
generateMandelbrot(
    complex float upperLeft, 
    complex float lowerRight, 
    int maxIterations, 
    int width, 
    int height)
{
    // Allocate image buffer, row-major order, 3 channels.
    unsigned char *image = malloc(height * width * 3);
    complex float cur = upperLeft; // Der Ausgangspunkt
    float dx = (crealf(lowerRight) - crealf(upperLeft))/width;  // die "Schrittgröße" für eine x-Iteration
    float dy = (cimagf(lowerRight) - cimagf(upperLeft))/height; // die "Schrittgröße" für eine y-Iteration

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
			// komplexe Zahl für diesen Pixel berechnen
			complex float c = dx*x + (dy*y)*I;
			c = complex_add(c, cur);
			
			// Mandelbrotfolge für diese Zahl durchgehen
			int index = testEscapeSeriesForPoint(c, maxIterations, 0);
			
			// Pixel einfärben
            int offset = (y * width + x) * 3;
            colorMapYUV(index, maxIterations, image + offset);
        }
    }

    return image;
}

