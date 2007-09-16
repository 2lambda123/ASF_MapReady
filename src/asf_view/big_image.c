#include "asf_view.h"
#include <gdk/gdkkeysyms.h>
#include "libasf_proj.h"

UserPolygon g_poly;

// current sizes of the large image.
// keep half the size around, we need that during the redraw, which
// is supposed to be quick
static int big_img_width=800;
static int big_img_height=800;
static int big_img_width2=400;
static int big_img_height2=400;

int get_big_image_width()
{
    return big_img_width;
}

int get_big_image_height()
{
    return big_img_height;
}

int get_big_image_width2()
{
    return big_img_width2;
}

int get_big_image_height2()
{
    return big_img_height2;
}

SIGNAL_CALLBACK void on_big_image_resize(GtkWidget *w, 
    GtkAllocation *alloc, gpointer user_data)
{
    // User resized!
    // Recalculate the width/height and half-width/half-height values
    // This if statement just prevents doing the redraw if the user didn't
    // actually resize (i.e., just grabbed the edge but didn't drag it).
    // (It also is useful to stop an infinate cascade-of-resizes that GTK
    // sometimes seems to get stuck in -- the "fill_small/big" will
    // occasionally trigger a resize that runs after this method completes)
    if (big_img_width != alloc->width || big_img_height != alloc->height)
    {
        big_img_width = alloc->width;
        big_img_height = alloc->height;
        big_img_width2 = big_img_width/2;
        big_img_height2 = big_img_height/2;

        fill_small();
        fill_big();
    }
}

// I am not sure this is even necessary
SIGNAL_CALLBACK void on_big_image_repaint(GtkWidget *w)
{
    fill_small();
    fill_big();
}

static void ls2img(double line, double samp, int *x, int *y)
{
    // convert from line/sample coordinates (SAR image coordinates)
    // to screen coordinates
    *x = (samp - (double)center_samp)/zoom + get_big_image_width2();
    *y = (line - (double)center_line)/zoom + get_big_image_height2();
}

static void img2ls(int x, int y, double *line, double *samp)
{
    // convert from screen coordinates to line/sample coordinates
    *line = ((double)y - get_big_image_height2())*zoom + (double)center_line;
    *samp = ((double)x - get_big_image_width2())*zoom + (double)center_samp;
}

static void destroy_pb_data(guchar *pixels, gpointer data)
{
    free(pixels);
}

static void show_or_hide_save_subset_button()
{
    // we show it when there is a user polygon defined
    // that's when saving a subset would make sense
    show_widget("save_subset_button", g_poly.n > 0);
}

// draws a crosshair at x,y (image coords)
static void put_crosshair (GdkPixbuf *pixbuf, double line, double samp, int green)
{
    if (samp < 0 || line < 0 || samp >= ns || line >= nl)
        return;

    int i, lo, hi;
    int width, height, rowstride, n_channels;
    guchar *pixels, *p;
    
    n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    
    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
    g_assert (!gdk_pixbuf_get_has_alpha (pixbuf));
    g_assert (n_channels == 3);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    
    // convert from image coords to screen coords
    int ix, iy;
    ls2img(line, samp, &ix, &iy);

    if (ix <= 0 || ix > width || iy <= 0 || iy > height)
        return;

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    // 15 is the size of the crosshair in each direction (30 pixels total)
    // these are screen pixels, so the crosshair is the same for all zooms
    lo = ix - 15;   if (lo < 0)      lo = 0;
    hi = ix + 15;   if (hi >= width) hi = width-1;

    // crosshair is drawn in red, unless "green" was set
    int r = 255;
    int g = 0;
    int b = 0;
    if (green) {
        r = b = 0;
        g = 255;
    }

    for (i = lo; i < hi; ++i) {
        if (i > ix-3 && i < ix+3) i = ix+3;
        p = pixels + iy * rowstride + i * n_channels;
        p[0] = r;
        p[1] = g;
        p[2] = b;
    }

    lo = iy - 15;   if (lo < 0)       lo = 0;
    hi = iy + 15;   if (hi >= height) hi = height-1;

    for (i = lo; i < hi; ++i) {
        if (i > iy-3 && i < iy+3) i = iy+3;
        p = pixels + i * rowstride + ix * n_channels;
        p[0] = r;
        p[1] = g;
        p[2] = b;
    }
}

static int iabs(int i)
{
    return i<0 ? -i : i;
}

// colors supported by put_line
#define RED 1
#define PURPLE 2

static void put_line(GdkPixbuf *pixbuf, double line0, double samp0, 
                     double line1, double samp1, int color)
{
    if (samp0 < 0 || line0 < 0 || samp1 < 0 || line1 < 0 ||
        samp0 >= ns || samp1 >= ns || line0 >= nl || line1 >= nl)
        return;

    int i, j, width, height, rowstride, n_channels;
    guchar *pixels, *p;
    
    n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    
    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
    g_assert (!gdk_pixbuf_get_has_alpha (pixbuf));
    g_assert (n_channels == 3);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    
    // convert from image coords to screen coords
    int ix0, iy0, ix1, iy1;
    ls2img(line0, samp0, &ix0, &iy0);
    ls2img(line1, samp1, &ix1, &iy1);

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    // color of the drawn line
    unsigned char r, g, b;
    if (color==RED) {
        r = 255;
        g = b = 0;
    } else if (color==PURPLE) {
        r = b = 255;
        g = 0;
    } else {
        assert(0);
        r=g=b=0;//not reached
    }

    // What a mess!  But the concept is trivial:
    //   (1) Loop goes on the x or y
    //   (2) linearly interpolate to find the other coordinate
    //   (3) place that pixel
    // i,j is the interpolated sample,line where the pixel goes
    // we loop on whichever distance (x or y) is greater
    if (iabs(ix1-ix0) > iabs(iy1-iy0)) {
        int incr = ix1>ix0 ? 1 : -1;
        for (i=ix0; i!=ix1; i+=incr) {
            j = iy0 + (float)(i-ix0)/(ix1-ix0) * (iy1-iy0);
            if (j >= 0 && i >= 0 && i <= width && j <= height) {
              p = pixels + j * rowstride + i * n_channels;
              p[0] = r; p[1] = g; p[2] = b;
            }
        }
    } else {
        int incr = iy1>iy0 ? 1 : -1;
        for (j=iy0; j!=iy1; j+=incr) {
            i = ix0 + (float)(j-iy0)/(iy1-iy0) * (ix1-ix0);
            if (j >= 0 && i >= 0 && i <= width && j <= height) {
              p = pixels + j * rowstride + i * n_channels;
              p[0] = r; p[1] = g; p[2] = b;
            }
        }
    }
}

static GdkPixbuf * make_big_image()
{
    assert(data_ci && meta);

    int ii, jj;
    int nchan = 3; // RGB for now, don't support RGBA yet
    int biw = get_big_image_width();
    int bih = get_big_image_height();
    unsigned char *bdata = MALLOC(sizeof(unsigned char)*biw*bih*nchan);

    int mm = 0;
    for ( ii = 0 ; ii < bih; ii++ ) {
        for ( jj = 0 ; jj < biw; jj++ ) {
            double l, s;
            img2ls(jj,ii,&l,&s);

            unsigned char r, g, b;

            if (l<0 || l>=nl || s<0 || s>=ns) {
                r = g = b = 0;
            } else {
                cached_image_get_rgb(data_ci, (int)floor(l),
                    (int)floor(s), &r, &g, &b);
            }

            int n = 3*mm;
            bdata[n] = r;
            bdata[n+1] = g;
            bdata[n+2] = b;
            ++mm;
        }
    }

    // Create the pixbuf
    GdkPixbuf *pb =
        gdk_pixbuf_new_from_data(bdata, GDK_COLORSPACE_RGB, FALSE, 
                                 8, biw, bih, biw*3, destroy_pb_data, NULL);
    
    if (!pb)
        asfPrintError("Failed to create the larger pixbuf.\n");

    // put the red crosshair at the "active" point along the polygon
    if (g_poly.c < g_poly.n)
        put_crosshair(pb, g_poly.line[g_poly.c], g_poly.samp[g_poly.c],
            FALSE);

    // draw the polygon
    if (g_poly.n > 0) {
        put_line(pb, crosshair_line, crosshair_samp,
            g_poly.line[0], g_poly.samp[0], RED);
        for (ii=0; ii<g_poly.n-1; ++ii) {
            put_line(pb, g_poly.line[ii], g_poly.samp[ii],
                g_poly.line[ii+1], g_poly.samp[ii+1], RED);
        }
    } else if (g_poly.show_extent) {
        // no polygon -- close "save subset" window, if open
        show_widget("save_subset_window", FALSE);
        g_poly.show_extent = FALSE;
    }

    // green crosshair goes second, so if the two overlap, we will see
    // the green one (the main one)
    put_crosshair(pb, crosshair_line, crosshair_samp, TRUE);

    // draw bounding box if requested
    if (g_poly.show_extent) {
        update_poly_extents();
        put_line(pb, g_poly.extent_y_min, g_poly.extent_x_min,
                     g_poly.extent_y_max, g_poly.extent_x_min, PURPLE);
        put_line(pb, g_poly.extent_y_max, g_poly.extent_x_min,
                     g_poly.extent_y_max, g_poly.extent_x_max, PURPLE);
        put_line(pb, g_poly.extent_y_max, g_poly.extent_x_max,
                     g_poly.extent_y_min, g_poly.extent_x_max, PURPLE);
        put_line(pb, g_poly.extent_y_min, g_poly.extent_x_max,
                     g_poly.extent_y_min, g_poly.extent_x_min, PURPLE);
    }

    return pb;
}

void fill_big()
{
    GdkPixbuf *pb = make_big_image();
    GtkWidget *img = get_widget_checked("big_image");
    gtk_image_set_from_pixbuf(GTK_IMAGE(img), pb);

    // might as well do this here
    show_or_hide_save_subset_button();
}

SIGNAL_CALLBACK int on_small_image_eventbox_button_press_event(
    GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    // clicking in the small image moves the big image
    GtkWidget *img = get_widget_checked("small_image");
    GdkPixbuf *pb = gtk_image_get_pixbuf(GTK_IMAGE(img));

    int w = gdk_pixbuf_get_width(pb);
    int h = gdk_pixbuf_get_height(pb);

    center_samp = ((int)event->x * ns) / (double)w;
    center_line = ((int)event->y * nl) / (double)h;

    fill_small();
    fill_big();

    return TRUE;
}

// Keeps track of which crosshair should be affected when the user
// presses an arrow key.
static int last_was_crosshair = TRUE;

SIGNAL_CALLBACK int
on_big_image_eventbox_button_press_event(
    GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    if (event->button == 1) {
        // ctrl-left-click: measure distance
        if (((int)event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
            if (g_poly.n < MAX_POLY_LEN) {
                double l, s;
                img2ls((int)event->x, (int)event->y, &l, &s);
                g_poly.line[g_poly.n] = l;
                g_poly.samp[g_poly.n] = s;
                ++g_poly.n;
                if (g_poly.n == 1)
                    g_poly.c = 0;
                else
                    g_poly.c = g_poly.n-1;
            } else {
                asfPrintWarning("Exceeded maximum polygon length.\n"
                                "No more points can be added.\n");
            }
            last_was_crosshair = FALSE;
        }
        // left-click: move crosshair
        else {
            img2ls((int)event->x, (int)event->y, &crosshair_line, &crosshair_samp);
            last_was_crosshair = TRUE;
        }
        update_pixel_info();
        fill_big();
    } else if (event->button == 3) {
        // right-click: re-center
        img2ls((int)event->x, (int)event->y, &center_line, &center_samp);
        fill_small();
        fill_big();
    }

    return TRUE;
}

void update_zoom()
{
   char buf[256];
   if (zoom >= 1)
       sprintf(buf, "Zoom: %.1f X", zoom);
   else if (zoom > .1)
       sprintf(buf, "Zoom: %.2f X", zoom);
   else if (zoom >= .01)
       sprintf(buf, "Zoom: %.3f X", zoom);
   else if (zoom >= .001)
       sprintf(buf, "Zoom: %.4f X", zoom);
   else if (zoom >= .0001)
       sprintf(buf, "Zoom: %.5f X", zoom);
   else
       sprintf(buf, "Zoom: %f X", zoom);
   put_string_to_label("zoom_label", buf);
}

static void zoom_in()
{
    // zooming in when larger than 1:1 decreases by 1 each time
    // until 1:1, then we halve the zoom factor
    if (zoom > 1)
        zoom = (int)(zoom+.98) - 1;
    else // zoom <= 1
        zoom /= 2;

    update_zoom();
    fill_small();
}

static void zoom_out()
{
    // zooming out when larger than 1:1 increases by 1 each time
    // when less than 1:1, then we double the zoom factor
    if (zoom <= 1)
        zoom *= 2;
    else // zoom > 1
        zoom = (int)zoom + 1;

    update_zoom();
    fill_small();
}

static void zoom_default()
{
    zoom = 1;
    update_zoom();
    fill_small();
}

SIGNAL_CALLBACK int
on_big_image_scroll_event(
    GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
    if (event->direction == GDK_SCROLL_UP) {
        zoom_in();
    } else if (event->direction == GDK_SCROLL_DOWN) {
        zoom_out();
    }

    fill_big();
    return TRUE;
}

static int handle_keypress(GdkEventKey *event)
{
    // handle the non-cursor-moving events first
    if (event->keyval == GDK_Page_Up || 
        event->keyval == GDK_Prior || 
        event->keyval == GDK_plus)
    {
        // Page Up or Plus: Zoom IN
        zoom_in();
    } else if (event->keyval == GDK_Page_Down || 
        event->keyval == GDK_Next || 
        event->keyval == GDK_minus)
    {
        // Page Down or Minus: Zoom OUT
        zoom_out();
    } else if (event->keyval == GDK_Home) {
        // Home: Revert to the normal zoom level
        zoom_default();
    } else if (event->keyval == GDK_End) {
        // End: Fit image to window
        int h = get_big_image_height();
        int w = get_big_image_width();

        // choose the larger of the horiz/vert zoom -- round up!
        //int z1 = (int)((double)nl/(double)h + .95);
        //int z2 = (int)((double)ns/(double)w + .95);
        double z1 = (double)nl/(double)h;
        double z2 = (double)ns/(double)w;
        zoom = z1 > z2 ? z1 : z2;

        // recenter the image as well
        center_line = nl/2;
        center_samp = ns/2;

        update_zoom();
        fill_small();
    } else if (event->keyval == GDK_Tab) {
        // Tab key: Cycle between the crosshairs (which one is affected by
        // subsequent arrow movements) Shift- or ctrl-tab: other direction
        if (g_poly.n > 0) {
            if (event->state & GDK_SHIFT_MASK ||
                event->state & GDK_CONTROL_MASK)
            {
                if (g_poly.c == 0) {
                    last_was_crosshair = TRUE;
                    g_poly.c = -1;
                } else if (g_poly.c == -1) {
                    last_was_crosshair = FALSE;
                    g_poly.c = g_poly.n - 1;
                } else {
                    last_was_crosshair = FALSE;
                    --g_poly.c;
                }
            } else {
                if (g_poly.c == g_poly.n-1) {
                    last_was_crosshair = TRUE;
                    g_poly.c = -1;
                } else {
                    last_was_crosshair = FALSE;
                    ++g_poly.c;
                }
            }
        } else
            last_was_crosshair = TRUE;
    } else if (event->keyval == GDK_BackSpace) {
        // Backspace: clear most recently added point
        if (g_poly.n > 0) {
            --g_poly.n;
            if (g_poly.c >= g_poly.n)
                g_poly.c = g_poly.n-1;
            update_pixel_info();
        }
    } else if (event->keyval == GDK_Escape) {
        // Escape: clear the ctrl-clicked path
        g_poly.n = g_poly.c = 0;
        update_pixel_info();
    } else if (event->keyval == GDK_c) {
        // c: Center image view on crosshair
        if (event->state & GDK_CONTROL_MASK && g_poly.n > 0 && g_poly.c >= 0) {
            center_line = g_poly.line[g_poly.c];
            center_samp = g_poly.samp[g_poly.c];
        } else {
            if (crosshair_line > 0 && crosshair_samp > 0) {
                center_line = crosshair_line;
                center_samp = crosshair_samp;
            } else {
                // I am not sure it is possible to get in here
                // we should always have a green crosshair
                center_line = crosshair_line = nl/2;
                center_samp = crosshair_samp = ns/2;
                last_was_crosshair = TRUE;
            }
        }
        fill_small();
    } else if (event->keyval == GDK_a) {
        // a: print out pixel values in a neighborhood of the
        //    crosshair.  ctrl-a uses the other crosshair
        int line=crosshair_line, samp=crosshair_samp;
        if (event->state & GDK_CONTROL_MASK) {
            if (g_poly.c < g_poly.n) {
                line = g_poly.line[g_poly.c];
                samp = g_poly.samp[g_poly.c];
            } else {
                line = samp = -1;
            }
        }
        if (line > 0 && samp > 0) {
            int i,j;
            printf("     |");
            for (j=samp-4; j<=samp+4; ++j)
                printf("%5d  ",j);
            printf("\n-----+");
            for (j=samp-4; j<=samp+4; ++j)
                printf("-------");
            printf("-\n");
            for (i=line-4; i<=line+4; ++i) {
                printf("%5d|",i);
                for (j=samp-4; j<=samp+4; ++j) {
                    if (i>=0 && j>=0 && i<nl && j<ns)
                        if (i==line && j==samp)
                            printf("%6.1f*",
                                cached_image_get_pixel(data_ci,i,j));
                        else
                            printf("%6.1f ",
                                cached_image_get_pixel(data_ci,i,j));
                    else
                        printf("     * ");
                }
                printf("\n");
            }
            printf("\n");
        }
        return TRUE;
    } else if (event->keyval == GDK_g) {
        // g: open google earth
        open_google_earth();
        return TRUE;
    } else if (event->keyval == GDK_m) {
        // m: open a new file
        open_mdv();
        return TRUE;
    } else if (event->keyval == GDK_s && event->state & GDK_CONTROL_MASK) {
        // ctrl-s: save subset
        if (g_poly.n > 0)
            save_subset();
        return TRUE;
    } else if (event->keyval == GDK_l) {
        // l: move to a local maxima (30x30 pixel search area)
        //    if ctrl-l is clicked, search 300x300 area
        // affects the same crosshair that would be moved with
        // the arrow keys
        int line, samp;
        if (!last_was_crosshair && g_poly.c < g_poly.n) {
            line = g_poly.line[g_poly.c]; samp=g_poly.samp[g_poly.c];
        } else {
            line = crosshair_line; samp=crosshair_samp;
        }
        int line_max=line, samp_max=samp;
        float max_val=-99999;
        int i,j,radius=15;
        if (event->state & GDK_CONTROL_MASK)
            radius*=10;
        for (i=line-radius; i<=line+radius; ++i) {
            for (j=samp-radius; j<=samp+radius; ++j) {
                if (i>=0 && i>=0 && i<nl && j<ns) {
                    float val = cached_image_get_pixel(data_ci,i,j);
                    if (val>max_val) {
                        max_val = val;
                        line_max = i;
                        samp_max = j;
                    }
                }
            }
        }
        if (!last_was_crosshair && g_poly.c < g_poly.n) {
            g_poly.line[g_poly.c]=line_max;
            g_poly.samp[g_poly.c]=samp_max;
        } else {
            crosshair_line=line_max;
            crosshair_samp=samp_max;
        }
        update_pixel_info();
    } else if (event->keyval == GDK_n) {
        // n: open a new file
        new_file();
        return TRUE;
    } else {
        // arrow key event (or a key we don't handle)
        // moves the crosshair (or ctrl-click crosshair) the specified
        // direction.  ctrl-arrow and shift-arrow will move the
        // arrow farther
        int z = (int)zoom;
        if (z==0) z=1; // z will be 0 when the zoom factor is <= .5

        int incr = z;

        // ALT: Recenters, instead of moves the crosshair
        // CTRL: Multiplies the movement amount by 10.
        // SHIFT: Multiplies the movement amount by 25.
        // So, CTRL-SHIFT will multiply the movement amount by 250

        if (event->state & GDK_CONTROL_MASK)
            incr *= 10;
        if (event->state & GDK_SHIFT_MASK)
            incr *= 25;

        if (event->state & GDK_MOD1_MASK) {
            // Alt was pressed --> move center
            switch (event->keyval) {
                case GDK_Up: center_line -= incr; break;
                case GDK_Down: center_line += incr; break;
                case GDK_Left: center_samp -= incr; break;
                case GDK_Right: center_samp += incr; break;
                default: return TRUE;
            }
            fill_small();
        } else {
            // Move one of the crosshairs
            if (last_was_crosshair) {
                switch (event->keyval) {
                    case GDK_Up: crosshair_line -= incr; break;
                    case GDK_Down: crosshair_line += incr; break;
                    case GDK_Left: crosshair_samp -= incr; break;
                    case GDK_Right: crosshair_samp += incr; break;
                    default: return TRUE;
                }
            } else {
                switch (event->keyval) {
                    if (g_poly.c < g_poly.n) {
                        case GDK_Up:
                            g_poly.line[g_poly.c] -= incr;
                            break;
                        case GDK_Down:
                            g_poly.line[g_poly.c] += incr; 
                            break;
                        case GDK_Left:
                            g_poly.samp[g_poly.c] -= incr; 
                            break;
                        case GDK_Right:
                            g_poly.samp[g_poly.c] += incr; 
                            break;
                    }
                    default: return TRUE;
                }
            }
            update_pixel_info();
        }
    }

    fill_big();
    return TRUE;
}

SIGNAL_CALLBACK int
on_big_image_eventbox_key_press_event(
    GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    // this event never seems to fire -- all kepresses are grabbed
    // by the "main_window" keypress handler below
    return handle_keypress(event);
}

SIGNAL_CALLBACK int
on_ssv_main_window_key_press_event(
    GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    return handle_keypress(event);
}

//  Haven't gotten the motion events firing yet... not sure what
//  to do if we do get them working, either
//SIGNAL_CALLBACK int
//on_big_image_eventbox_motion_notify_event(
//    GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
//{
//    printf("motion...\n");
//
//    double line, samp;
//    img2ls((int)event->x, (int)event->y, &line, &samp);
//
//    // now do something cool!!
//
//    return TRUE;
//}
