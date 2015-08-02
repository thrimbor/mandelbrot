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
#include <stdlib.h>
#include <stdio.h>

#include "ppm.h"

void exportPPM(const char *filename, struct PPM *image)
{
	FILE *img;
    img = fopen(filename, "w");
    if (img == NULL) {
        printf("Error saving image!\n");
        return;
    }

    fprintf(img, "P3 %d %d 255 ", image->width, image->height);;
    for(int y = image->height - 1; y >= 0; y--) {
        for(int x = 0; x < image->width * 3; x++) {
            fprintf(img, "%d\n",(int)(image->data[y * image->width * 3 + x]));
		}
	}
	fclose(img);
}
