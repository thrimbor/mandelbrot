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
#include <CL/opencl.h>

cl_kernel kernel;
cl_command_queue queue;
cl_context context;

void initMandelbrot ()
{
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, device, 0, NULL);

    char *source = NULL;
    FILE *fp = fopen("mandelbrot.cl", "r");
    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0) {
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }
            source = malloc(sizeof(char) * (bufsize + 1));
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (newLen == 0) {
                fputs("Error reading file", stderr);
            } else {
                source[newLen] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }
    const char* sourceptr = source;
    cl_program program = clCreateProgramWithSource(context, 1, &sourceptr, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "mandelbrot", NULL);
    free(source);
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
    cl_mem buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 1024*768*3, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    //static const int width = 1024;
    clSetKernelArg(kernel, 1, sizeof(width), &width);
    static const float radius = 2.0f;
    clSetKernelArg(kernel, 2, sizeof(radius), &radius);
    clSetKernelArg(kernel, 3, sizeof(maxIterations), &maxIterations);

    const size_t globalWorkSize[] = {1024, 768, 0, 0};
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);

    unsigned char *image = malloc(height * width * 3);

    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, 1024*768*3, image, 0, NULL, NULL);
    return image;
}

