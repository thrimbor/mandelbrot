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
#ifndef PPM_HEADER
#define PPM_HEADER

/*
 * Representation of an image.
 */
struct PPM {
	int width;
	int height;
	unsigned char* data;
};

/*
 * Saves a PPM file (3-channels, each 8-bit unsigned char) to disk into a file named path.
 * 
 * Arguments:
 *	filename - The filename including path for the image file.
 *	image - PPM structure containing width, height and actual data of an image.
 */
void
exportPPM(const char *filename, struct PPM *image);

#endif /* PPM_HEADER */
