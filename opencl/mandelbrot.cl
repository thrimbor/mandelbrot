float2 complex_sqr (const float2 a)
{
	return (float2) (a.x*a.x - a.y*a.y, a.x*a.y + a.y*a.x);
}

__kernel void mandelbrot (__global uchar* outImage, const int width, const float radius, const int iterations)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	float2 z = (float2)(0.0f, 0.0f);
	float2 c = (float2)(-2.5f, 1.5f);
	c += (float2)(x*(1-(-2.5f))/width, y*(-1.5f-1.5f)/768);
	int i=0;
	
	while ((length(z) <= (radius)) && (i<iterations))
	{
		z = complex_sqr(z) + c;
		i++;
	}
	
	/*
	if (i < iterations)
	{
		i += 1.0f - (log(log(length(z)) / log(2.0f)) / log(2.0f));
	}
	*/
	
	
	
	// colormapping
	if (i == iterations)
	{
		outImage[(y*width + x)*3 + 0] = 0;
		outImage[(y*width + x)*3 + 1] = 0;
		outImage[(y*width + x)*3 + 2] = 0;
	}
	else
	{
		outImage[(y*width + x)*3 + 0] = 0;
		outImage[(y*width + x)*3 + 1] = (i*255)/iterations;
		outImage[(y*width + x)*3 + 2] = 0;
	}
}
