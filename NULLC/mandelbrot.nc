import complex;
import win.window;
import img.canvas;
import std.io;


int testEscapeSeriesForPoint (Complex c, int maxIterations)
{
	Complex z = Complex(0.0f, 0.0f);
	int iteration = 0;
	
	while ((z.abs() <= 2) && (iteration < maxIterations))
	{
		z = (z*z)+c;
		iteration++;
	}
	
	return iteration;
}



Window main = Window("Mandelbrot", 40, 30, 1024 + 4, 768 + 20);
Canvas img = Canvas(1024, 768);

Complex cur = Complex(-2.5, 1.5);
float dx = (1 - (-2.5))/1024;
float dy = (-1.5 - 1.5)/768;

img.Clear(0, 0, 0);

int maxIterations = 15;

for (int y=0; y<768; y++)
{
	for (int x=0; x<1024; x++)
	{
		Complex c = Complex(dx*x, dy*y);
		c = cur+c;
		
		int it = testEscapeSeriesForPoint(c, maxIterations);
		
		
		if (it == maxIterations)
		{
			img.SetColor(0,0,0);
		} else {
			float y,u,v;
			y = 0.2f;
			u = -1.0f + 2.0f * (float(it) / float(maxIterations));
			v = 0.5f - (float(it) / float(maxIterations));
			
			float r,g,b;
			r = y + 1.28033f*v;
        	g = y - 0.21482f*u - 0.38059f*v;
        	b = y + 2.12798f*u;
			
			img.SetColor(r*255, g*255, b*255);
		}
		
		img.DrawPoint(x,y);
		
		
		
	}
}

char[256] keys;

do
{
	main.DrawCanvas(&img, 0, 0);
	main.Update();
	GetKeyboardState(keys);
} while(!(keys[0x1B] & 0x80000000));

