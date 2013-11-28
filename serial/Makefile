CC = gcc

COMMON_C_FLAGS = -Wall -std=c99 -O3 -msse3
COMMON_LD_FLAGS = -lm

CLI_C_FLAGS = $(COMMON_C_FLAGS)
CLI_LD_FLAGS = $(COMMON_LD_FLAGS)

GUI_C_FLAGS = $(COMMON_C_FLAGS) `pkg-config --cflags gtk+-2.0` -pthread
GUI_LD_FLAGS = $(COMMON_LD_FLAGS) `pkg-config --libs gtk+-2.0` -lpthread

cli: lib
	$(CC) $(CLI_C_FLAGS) -c CLI.c $(CLI_LD_FLAGS)
	$(CC) $(CLI_C_FLAGS) -o mandelbrot_cli mandelbrot.o ppm.o CLI.o $(CLI_LD_FLAGS)
	@echo "-->" Generated mandelbrot_cli. Type \"./mandelbrot_cli\" to execute.

gui: lib
	$(CC) $(GUI_C_FLAGS) -c GUI.c $(GUI_LD_FLAGS)
	$(CC) $(GUI_C_FLAGS) -o mandelbrot_gui mandelbrot.o GUI.o $(GUI_LD_FLAGS)
	@echo "-->" Generated mandelbrot_gui. Type \"./mandelbrot_gui\" to execute.

lib:
	$(CC) $(COMMON_C_FLAGS) -c mandelbrot.c $(COMMON_LD_FLAGS)
	$(CC) $(COMMON_C_FLAGS) -c ppm.c $(COMMON_LD_FLAGS)

clean:
	$(RM) mandelbrot_cli
	$(RM) mandelbrot_gui
	$(RM) *.o
