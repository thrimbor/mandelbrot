/*
 *   Mandelbrot Set Viewer - Simple CLI
 *   Copyright (C) 2013 Daniel Thürck
 
 *   This program is free software; you can redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.

 *   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License along with this program;
 *   if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <getopt.h>

#include "globals.h"
#include "mandelbrot.h"
#include "ppm.h"

struct arguments {
    int maxIterations;
    char * outfile;
};

static struct option long_options[] = {
    {"maxiterations", required_argument, 0, 'i'},
    {"outfile", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

void
help(void)
{
    printf("USAGE: mandelbrot_cli [OPTIONS]\n");
    printf("With [OPTIONS]:\n");
    printf("\t -i --maxiterations INT \t maximum number of series iterations per pixel\n");
    printf("\t -o --outfile FILE \t filename (without appendix) for output picture in PPM format\n");
    printf("\n");
}

int
main(int argc, char *argv[])
{
    // for timings
    struct timeval start;
    struct timeval stop;

    // argument parsing
    struct arguments args;
    args.maxIterations = 100;
    args.outfile = "mandelbrot.ppm";
    int c = 0;

    while(1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "hi:o:", long_options, &option_index);

        // exit loop after parsing all arguments
        if(c == -1) {
            break;
        }

        switch(c) {
            case 'h':
                help();
                exit(0);
                break;
            case 'i':
                args.maxIterations = atoi(optarg);
                if(args.maxIterations < 2) {
                    args.maxIterations = 2;
                }
                if(args.maxIterations > 10000) {
                    args.maxIterations = 10000;
                }
                printf("Maximum iterations: %d\n", args.maxIterations);
                break;
            case 'o':
                args.outfile = optarg;
                printf("Result file: %s\n", args.outfile);
                break;
            default:
                printf("Unknown option, terminating...\n");
                exit(-1);
        }
    }

    gettimeofday(&start, 0);
    unsigned char *data = generateMandelbrot(INITIAL_UPPERLEFT, INITIAL_LOWERRIGHT, args.maxIterations, WIDTH, HEIGHT);
    gettimeofday(&stop, 0);

    long renderTime = (stop.tv_sec-start.tv_sec)*1000 + (stop.tv_usec-start.tv_usec)/1000;
    printf("Rendering took %ld ms...\n", renderTime);

    printf("Writing image...\n");
    struct PPM image;
    image.width = WIDTH;
    image.height = HEIGHT;
    image.data = data;
    exportPPM(args.outfile, &image);

    return 0;
}

