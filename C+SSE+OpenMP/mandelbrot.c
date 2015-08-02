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
#include <xmmintrin.h> // SSE 1
#include <emmintrin.h> // SSE 2
#include <pmmintrin.h> // SSE 3

/*
 * Multipliziert zwei komplexe Zahlen mit zwei komplexen Zahlen
 *
 * Arguments:
 *  a - die beiden linken Faktoren
 *  b - die beiden rechten Faktoren
 * 
 * Returns:
 *  Die beiden Produkte
 */
__attribute__ ((hot)) static inline __m128 complex_mul (__m128 a, __m128 b)
{
	// Registerinhalt:
	// a = [ imag_2 | real_2 | imag | real ]
	// b = [ imag_2 | real_2 | imag | real ]
	
	// Zuerst aus a [ a_real2 | a_real2 | a_real | a_real ] machen (_mm_moveldup_ps)
	// dann mit b multiplizieren (_mm_mul_ps)
	// heraus kommt: [ a_real2 * b_imag2 | a_real2 * b_real2 | a_real * b_imag | a_real * b_real ]
	
	// Aus a [ a_imag2 | a_imag2 | a_imag | a_imag ] machen (_mm_movehdup_ps)
	// dann mit [ b_real2 | b_imag2 | b_real | b_imag ] multiplizieren, welches aus b geshuffled wird
	// heraus kommt: [ a_imag2 * b_real2 | a_imag2 * b_imag2 | a_imag * b_real | a_imag * b_imag ]
	
	// Die beiden Teilergebnisse werden dann mit addsub verrechnet (0 und 2 werden subtrahiert, 1 und 3 werden addiert
	
	return _mm_addsub_ps( _mm_mul_ps(_mm_moveldup_ps(a), b),
						  _mm_mul_ps(_mm_movehdup_ps(a), _mm_shuffle_ps(b,b, _MM_SHUFFLE(2, 3, 0, 1))));
}

/*
 * Berechnet die Quadrate der Beträge von zwei komplexen Zahlen
 *
 * Arguments:
 *  c - die komplexen Zahlen
 *  f - der Pointer in dem das Quadrat des Betrages der unteren komplexen Zahl abgelegt wird
 *  g - der Pointer in dem das Quadrat des Betrages der oberen komplexen Zahl abgelegt wird
 * 
 * Returns:
 *  Nichts
 */
__attribute__ ((hot)) static inline void complex_abs_sqr (__m128 c, float* f, float* g)
{
	__m128 m;
	
	m = _mm_mul_ps(c,c);               // vertikale Multiplikation, d.h. quadrieren [ imag2 | real2 | imag | real ] -> [ imag2*imag2 | real2*real2 | imag*imag | real*real ]
	m = _mm_hadd_ps(m,m);              // horizontale Addition [ imag2*imag2 | real2*real2 | imag*imag | real*real ] -> [ imag2*imag2+real2*real2 | imag*imag+real*real | imag2*imag2+real2*real2 | imag*imag+real*real ]
	_mm_store_ss(f, m);				   // unterstes Element in f speichern (imag*imag+real*real)
	_mm_store_ss(g, _mm_shuffle_ps(m,m, _MM_SHUFFLE(3,2,0,1))); // zweites Element in g speichern (imag2*imag2+real2*real2)
}

__m128 rgb_p, rgb_r, rgb_g, rgb_b;

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
__attribute__ ((hot)) static inline void 
colorMapYUV(int index, int maxIterations, unsigned char* color)
{
	// Wenn die komplexe Zahl Teil der Mandelbrotmenge ist, wird sie schwarz eingefärbt
	if (index == maxIterations)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		return;
	}
	
	// Farbschema anwenden
	__m128 rgb_c;
	__m128 i_maxi = _mm_set_ps(0.0f, 2.0f * (float)index / (float)maxIterations, -(float)index / (float)maxIterations, 0.0f);
	rgb_c = _mm_add_ps(rgb_p, i_maxi);
	// rgb_c enthält jetzt YUV
	
	// Mit Hilfe der globalen Konvertierungsmatrix den benötigten Anteil von Y, U und V pro Farbe ausrechnen
	__m128 red = _mm_mul_ps(rgb_r, rgb_c);
	__m128 green = _mm_mul_ps(rgb_g, rgb_c);
	__m128 blue = _mm_mul_ps(rgb_b, rgb_c);
	
	// Die Anteile aufsummieren
	red = _mm_hadd_ps(red,red);
	red = _mm_hadd_ps(red,red);
	green = _mm_hadd_ps(green,green);
	green = _mm_hadd_ps(green,green);
	blue = _mm_hadd_ps(blue,blue);
	blue = _mm_hadd_ps(blue,blue);
	
	// RGB-Werte auslesen und abspeichern
	float f __attribute__ ((aligned(16)));
	_mm_store_ss(&f, red);
	color[0] = f;
	_mm_store_ss(&f, green);
	color[1] = f;
	_mm_store_ss(&f, blue);
	color[2] = f;
}

// Der Logarithmus ist langsam, daher wird log(2) vorberechnet
float logof2 = 0.6931471806;

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
__attribute__ ((hot)) static inline void
testEscapeSeriesForPoint(__m128 c, int maxIterations, int* it1, int* it2)
{
	// Statt den Betrag der komplexen Zahl mit dem Radius zu vergleichen,
	// vergleichen wir das Quadrat des Betrags der komplexen Zahl mit dem Quadrat des Radius.
	// Dadurch sparen wir uns das Wurzelziehen in jeder Iteration (und Wurzeln sind langsam!)
	float r  = RADIUS*RADIUS;
	
	// Zwischenspeicher für die Quadrate der Beträge der komplexen Zahlen
	float f = 0;
	float g = 0;
	
    __m128 z = _mm_setzero_ps();
	
	// Die Iterationszähler
	int iteration1 = 0;
	int iteration2  = 0;
	
	// Hier merken wir uns ob wir mit einer Zahl schon fertig sind
	int it1_done  = 0;
	int it2_done  = 0;
	
	// Variablen für die "echten" Beträge fürs smooth coloring.
	float f_sqrt=0, g_sqrt=0;
	
	while (iteration1 < maxIterations && iteration2 < maxIterations)
	{
		// Quadrate der Beträge berechnen
		complex_abs_sqr(z, &f, &g);
		
		if ((f > r) && (g > r)) {
			// Wenn beide Zahlen den Radius verlassen haben müssen wir nicht mehr weiterrechnen
			break;
		} else if (f>r && !it1_done) {
			it1_done = -1;    // Mit der unteren Zahl sind wir fertig
			f_sqrt = sqrt(f); // Den "echten" Betrag berechnen, der wird noch für das smooth-coloring gebraucht
			
			// Wenn wir mit einer Zahl fertig sind wird die andere dupliziert und überschreibt erstere - ansonsten könnte erstere überlaufen, und das gäbe Probleme!
			z = _mm_shuffle_ps(z,z, _MM_SHUFFLE(3,2,3,2));
		} else if (g>r && !it2_done) {
			it2_done = -1;    // Mit der oberen Zahl sind wir fertig
			g_sqrt = sqrt(g); // Den "echten" Betrag berechnen, der wird noch für das smooth-coloring gebraucht
			
			// Wenn wir mit einer Zahl fertig sind wird die andere dupliziert und überschreibt erstere - ansonsten könnte erstere überlaufen, und das gäbe Probleme!
			z = _mm_shuffle_ps(z,z, _MM_SHUFFLE(1,0,1,0));
		}
		
		// Wenn wir mit beiden Zahlen fertig sind können wir ja aufhören
		if (it1_done && it2_done) break;
			
		// Mandelbrotfolge für n+1 berechnen
		z = _mm_add_ps(complex_mul(z,z), c);
		
		// Wenn wir mit dieser Zahl noch nicht fertig sind inkrementieren wir die Iterationsvariable
		if (!it1_done)
			iteration1++;
		
		// Wenn wir mit dieser Zahl noch nicht fertig sind inkrementieren wir die Iterationsvariable
		if (!it2_done)
			iteration2++;
	}
	
	// Wenn die untere komplexe Zahl nicht in der Mandelbrot-Menge liegt, smooth coloring anwenden
	if (iteration1 < maxIterations)
	{
		if (!it1_done) f_sqrt = sqrt(f);
		iteration1 += 1.0f - (log(log(f_sqrt) / logof2) / logof2);
	}
	
	// Wenn die obere komplexe Zahl nicht in der Mandelbrot-Menge liegt, smooth coloring anwenden
	if (iteration2 < maxIterations)
	{
		if (!it2_done) g_sqrt = sqrt(g);
		iteration2 += 1.0f - (log(log(g_sqrt) / logof2) / logof2);
	}
	
	// Iterationen zurückgeben
	*it1 = iteration1;
	*it2 = iteration2;
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
	// globale Variablen für Farbschema und YUV->RGB Konvertierung initialisieren
	rgb_p = _mm_set_ps(0.2f, -1.0f, 0.5f, 0.0f);
	rgb_r = _mm_set_ps(1.0f*255.0f, 0.0f, 1.28033f*255.0f, 0.0f);
	rgb_g = _mm_set_ps(1.0f*255.0f, -0.21482f*255.0f, -0.38059f*255.0f, 0.0f);
	rgb_b = _mm_set_ps(1.0f*255.0f, 2.12782f*255.0f, 0.0f, 0.0f);
	
    // Allocate image buffer, row-major order, 3 channels.
    unsigned char *image = malloc(height * width * 3);
    __m128 cur = _mm_set_ps(cimagf(upperLeft), crealf(upperLeft), cimagf(upperLeft), crealf(upperLeft)); // Der Ausgangspunkt, in doppelter Ausführung da wir zwei komplexe Zahlen auf einmal verarbeiten
    float dx = (crealf(lowerRight) - crealf(upperLeft))/width;   // die "Schrittgröße" für eine x-Iteration
    float dy = (cimagf(lowerRight) - cimagf(upperLeft))/height;  // die "Schrittgröße" für eine y-Iteration
    
    // Die for-Schleife wird mit OpenMP parallelisiert. Sollte das nicht erlaubt sein, kann man das im Makefile ausschalten - dann wird das #pragma einfach ignoriert
    #pragma omp parallel for schedule(dynamic)
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x+=2) {
			// komplexe Zahlen für zwei Pixel berechnen
			__m128 c = _mm_set_ps(dy*y, dx*(x+1), dy*y, dx*x);
			c = _mm_add_ps(c, cur);
			
			// Mandelbrotfolge für beide Zahlen durchgehen
			int index1,index2;
			testEscapeSeriesForPoint(c, maxIterations, &index1, &index2);
			
			// beide Pixel einfärben
            int offset = (y * width + x) * 3;
            colorMapYUV(index1, maxIterations, image + offset);
            colorMapYUV(index2, maxIterations, image+offset+3);
        }
    }

    return image;
}

