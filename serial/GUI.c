/*
 *   Mandelbrot Set Viewer - Simple GUI
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
#include <stdlib.h>
#include <sys/time.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pthread.h>

#include "globals.h"
#include "mandelbrot.h"

/* ---------------------------- Variables ----------------------------- */

// window
GtkWidget *winMain;
GString *strWinTitle;
GtkWidget *hLayout;
GtkWidget *tblLayout;
GtkWidget *imgSet;
GtkWidget *vsLine;
GtkWidget *aSpacer;
GtkWidget *evImageBox;

// timing info
GtkWidget *lblTime;
GtkWidget *lblTiming;

struct timeval start;
struct timeval stop;

// iteration number chooser
GtkWidget *lblMaxIterations;
GtkWidget *hscMaxIterations;
GtkObject *adjIterations;

// buttons
GtkWidget *bReset;
GtkWidget *bRender;
GtkWidget *hbButtons;

// threads
pthread_t t;

// functional values
complex float upperLeft = INITIAL_UPPERLEFT;
complex float lowerRight = INITIAL_LOWERRIGHT;
int iLevel = 0;
int iLevelMax = 16;
int maxIterations = 100;
gboolean rerender = FALSE;
gboolean rendering = FALSE;

unsigned char *buffer;
GdkPixbuf *image;

/* --------------------- Forward Declarations ------------------------- */

int setUpGUI(int, char **);
int setUpParameterChooser(int, char **);
int setUpCChooser(int, char **);
int setUpItChooser(int, char **);
int setUpButtons(int, char **);
void destroy(GtkWidget *, gpointer);
gboolean delete_event(GtkWidget *, GdkEvent *, gpointer);
GdkPixbuf * convertColorArray(unsigned char *);
void bRender_clicked(GtkWidget *, gpointer);
void bReset_clicked(GtkWidget *, gpointer);
void evImageBox_clicked(GtkWidget *, GdkEventButton *, gpointer);
void GUIrender(void);
void render(void);

/* -------------------------- Implementation -------------------------- */

int
setUpGUI(int argc, char *argv[])
{

    // create application main window
    winMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // setup title and size
    strWinTitle = g_string_new("GDI3P2: Mandelbrot-Fractal");
    gtk_window_set_title(GTK_WINDOW(winMain), strWinTitle->str);
    gtk_window_set_policy(GTK_WINDOW(winMain), FALSE, FALSE, FALSE);

    // setup outer layout
    imgSet = gtk_image_new();
    gtk_widget_set_size_request(imgSet, WIDTH, HEIGHT);
    hLayout = gtk_hbox_new(FALSE, 10);
    tblLayout = gtk_table_new(4, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(tblLayout), 40);
    vsLine = gtk_vseparator_new();
    evImageBox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(evImageBox), imgSet);
    g_signal_connect(G_OBJECT(evImageBox), "button_press_event", G_CALLBACK(evImageBox_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(hLayout), evImageBox, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hLayout), vsLine, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hLayout), tblLayout, FALSE, FALSE, 5);
    gtk_container_add(GTK_CONTAINER(winMain), hLayout);

    // setup GUI contents: controls for Mandelbrot parameter setting
    setUpParameterChooser(argc, argv);

    // connect signals for correct program termination on window close
    g_signal_connect(G_OBJECT(winMain), "destroy", G_CALLBACK(destroy), NULL);
    g_signal_connect(G_OBJECT(winMain), "delete_event", G_CALLBACK(destroy), NULL);

    // display window
    gtk_widget_show_all(winMain);

    return 1;
}

int
setUpParameterChooser(int argc, char *argv[])
{
    setUpItChooser(argc, argv);
    setUpButtons(argc, argv);

    lblTime = gtk_label_new("Computation time:");
    lblTiming = gtk_label_new("-- no reference --");

    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(lblMaxIterations), 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(hscMaxIterations), 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(lblTime), 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(lblTiming), 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(hbButtons), 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);

    aSpacer = gtk_alignment_new(0, 0, 1, 1);
    gtk_widget_set_size_request(aSpacer, 10, 500);
    gtk_table_attach(GTK_TABLE(tblLayout), GTK_WIDGET(aSpacer), 0, 2, 3, 4, GTK_SHRINK, GTK_EXPAND, 0, 0);

    return 1;
}

int
setUpItChooser(int argc, char *argv[])
{
    lblMaxIterations = gtk_label_new("Iterations:");
    
    adjIterations = gtk_adjustment_new(100, 10, 1000, 10, 100, 0);
    hscMaxIterations = gtk_hscale_new(GTK_ADJUSTMENT(adjIterations));
    gtk_widget_set_size_request(hscMaxIterations, 100, 30);
    gtk_scale_set_digits(GTK_SCALE(hscMaxIterations), 0);
    gtk_scale_set_value_pos(GTK_SCALE(hscMaxIterations), GTK_POS_RIGHT);

    return 1;
}

int 
setUpButtons(int argc, char *argv[]) 
{
    bReset = gtk_button_new_with_label("Reset");
    bRender = gtk_button_new_with_label("Render");

    // connect signals
    g_signal_connect(G_OBJECT(bRender), "clicked", G_CALLBACK(bRender_clicked), NULL);
    g_signal_connect(G_OBJECT(bReset), "clicked", G_CALLBACK(bReset_clicked), NULL);

    hbButtons = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(hbButtons), bReset, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbButtons), bRender, FALSE, FALSE, 5);

    return 1;
}

void
destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

gboolean
delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return FALSE;
}

GdkPixbuf * 
convertColorArray(unsigned char * colorMap)
{
    guchar * rawImage = (guchar*) malloc(WIDTH * HEIGHT * 3 * sizeof(guchar));
    int counter = 0;

    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            rawImage[counter++] = (guchar) colorMap[y*WIDTH*3+x*3+0];
            rawImage[counter++] = (guchar) colorMap[y*WIDTH*3+x*3+1];
            rawImage[counter++] = (guchar) colorMap[y*WIDTH*3+x*3+2];
        }
    }

    return gdk_pixbuf_new_from_data(rawImage, GDK_COLORSPACE_RGB, FALSE, 8, WIDTH, HEIGHT, WIDTH * 3 * sizeof(guchar), NULL, NULL);
}

void
bRender_clicked(GtkWidget * widget, gpointer data)
{
    maxIterations = (int) gtk_range_get_value(GTK_RANGE(hscMaxIterations));
    
    if(iLevel < iLevelMax) {
        GUIrender();
    }
}

void 
bReset_clicked(GtkWidget * widget, gpointer data)
{
    upperLeft = -2.5 + 1.5*I;
    lowerRight = 1 - 1.5*I;
    iLevel = 0;
    GUIrender();
}

void 
evImageBox_clicked(GtkWidget * widget, GdkEventButton *event, gpointer data)
{
    if(rerender && !rendering) {
        maxIterations = (int) gtk_range_get_value(GTK_RANGE(hscMaxIterations));
        
        float spanX = crealf(lowerRight) - crealf(upperLeft);
        float spanY = cimagf(upperLeft) - cimagf(lowerRight);

        float centerX = crealf(upperLeft) + event->x/WIDTH * spanX;
        float centerY = cimagf(lowerRight) + (HEIGHT - event->y)/HEIGHT * spanY;

        upperLeft = (centerX - ZOOM/2 * spanX) + (centerY + ZOOM/2 * spanY) * I;
        lowerRight = (centerX + ZOOM/2 * spanX) + (centerY - ZOOM/2 * spanY) * I;

        GUIrender();
    }
}

void
GUIrender(void)
{
    if(!rendering) {
        rendering = TRUE;

        gtk_widget_set_sensitive(GTK_WIDGET(bRender), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(bReset), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(hscMaxIterations), FALSE);
    
        if(rerender) {
            free(buffer);
            gtk_image_clear(GTK_IMAGE(imgSet));
            g_object_unref(G_OBJECT(image));
        }
		
		
		#ifdef USE_PTHREADS
		pthread_create(&t, NULL, &render, NULL);
		#else
		render();
		#endif
    }
}

void 
render(void)
{
	#ifdef USE_PTHREADS
	gdk_threads_enter();
	#endif
	
    gettimeofday(&start, NULL);
    buffer = generateMandelbrot(upperLeft, lowerRight, maxIterations, WIDTH, HEIGHT);
    image = convertColorArray(buffer);
    gettimeofday(&stop, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(imgSet), image);
    rerender = TRUE;

    gtk_widget_set_sensitive(GTK_WIDGET(bRender), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(bReset), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(hscMaxIterations), TRUE);

    long renderTime = (stop.tv_sec-start.tv_sec)*1000 + (stop.tv_usec-start.tv_usec)/1000;
    gtk_label_set_text(GTK_LABEL(lblTiming), g_strdup_printf("%ld ms", renderTime));
    rendering = FALSE;
    
    #ifdef USE_PTHREADS
	gdk_threads_enter();
	pthread_exit(0);
	#endif
}

int
main(int argc, char *argv[]) 
{
    gtk_init(&argc, &argv);
    int iError = setUpGUI(argc, argv);
    gtk_main();
    return iError;
}
