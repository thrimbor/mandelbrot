float2 complex_sqr (const float2 a)
{
	return (float2) (a.x*a.x - a.y*a.y, a.x*a.y + a.y*a.x);
}

void colorMapYUV (const float index, const float maxIterations, __global uchar* color)
{
	// Wenn die komplexe Zahl Teil der Mandelbrotmenge ist, wird sie schwarz eingefärbt
	if (index == maxIterations) {
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		return;
	}
    
	// Farbschema anwenden
    const float y = 0.2f;
    const float u = -1.0f + 2.0f * (index / maxIterations);
    const float v = 0.5f - (index / maxIterations);
	
	// YUV -> RGB
	const float r = y + 1.28033f*v;
	const float g = y - 0.21482f*u - 0.38059f*v;
	const float b = y + 2.12798f*u;
	
	// RGB-Werte ins Array schreiben, dabei den Wertebereich von [0;1] auf [0;255] skalieren
	color[0] = (char) (r*255.0f);
	color[1] = (char) (g*255.0f);
	color[2] = (char) (b*255.0f);
}

__kernel void mandelbrot (__global uchar* outImage, const int width, const int height, const float radius, const int iterations)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	float2 z = (float2)(0.0f, 0.0f);
	float2 c = (float2)(-2.5f, 1.5f);
	c += (float2)(x*(1-(-2.5f))/width, y*(-1.5f-1.5f)/height);
	int i=0;
	
	while ((length(z) <= (radius)) && (i<iterations))
	{
		z = complex_sqr(z) + c;
		i++;
	}
	
	if (i < iterations)
	{
		i += 1.0f - (log(log(length(z)) / log(2.0f)) / log(2.0f));
	}
	
	colorMapYUV(i, iterations, outImage+((y*width+x)*3));
}
