#include "ssv.h"

static void destroy_pb_data(guchar *pixels, gpointer data)
{
    free(pixels);
}

static void put_bounding_box(GdkPixbuf *pixbuf)
{
    int i, width, height, rowstride, n_channels;
    guchar *pixels, *p;
    const int bb_width = get_big_image_width();
    const int bb_height = get_big_image_height();

    n_channels = gdk_pixbuf_get_n_channels (pixbuf);
    
    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
    g_assert (!gdk_pixbuf_get_has_alpha (pixbuf));
    g_assert (n_channels == 3);
    
    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    int bb_min_x = (center_samp - bb_width*zoom/2) * width / ns;
    if (bb_min_x < 0) bb_min_x = 0;
    if (bb_min_x > width-1) bb_min_x = width-1;

    int bb_max_x = (center_samp + bb_width*zoom/2) * width / ns;
    if (bb_max_x < 0) bb_max_x = 0;
    if (bb_max_x > width-1) bb_max_x = width-1;

    int bb_min_y = (center_line - bb_height*zoom/2) * height / nl;
    if (bb_min_y < 0) bb_min_y = 0;
    if (bb_min_y > height-1) bb_min_y = height-1;

    int bb_max_y = (center_line + bb_height*zoom/2) * height / nl;
    if (bb_max_y < 0) bb_max_y = 0;
    if (bb_max_y > height-1) bb_max_y = height-1;

    for (i=bb_min_x; i<=bb_max_x; ++i) {
        p = pixels + bb_min_y * rowstride + i * n_channels;
        p[0] = 255;
        p[1] = p[2] = 0;
        p = pixels + bb_max_y * rowstride + i * n_channels;
        p[0] = 255;
        p[1] = p[2] = 0;
    }

    for (i=bb_min_y+1; i<bb_max_y; ++i) {
        p = pixels + i * rowstride + bb_min_x * n_channels;
        p[0] = 255;
        p[1] = p[2] = 0;
        p = pixels + i * rowstride + bb_max_x * n_channels;
        p[0] = 255;
        p[1] = p[2] = 0;
    }
}

GdkPixbuf *pixbuf_small = NULL;

static GdkPixbuf * make_small_image(int size, int force)
{
    if (!pixbuf_small || force) {
        if (pixbuf_small) {
            g_object_unref(pixbuf_small);
            pixbuf_small = NULL;
        }

        assert((data||data_ci) && meta);

        int larger_dim = size*4;
        if (larger_dim > meta->general->line_count)
            larger_dim = meta->general->line_count;

        //printf("Larger size: %d\n", larger_dim);
        int ii, jj;

        // Vertical and horizontal scale factors required to meet the
        // max_thumbnail_dimension part of the interface contract.
        int vsf = ceil (nl / larger_dim);
        int hsf = ceil (ns / larger_dim);
        // Overall scale factor to use is the greater of vsf and hsf.
        int sf = (hsf > vsf ? hsf : vsf);

        // Image sizes.
        size_t tsx = meta->general->sample_count / sf;
        size_t tsy = meta->general->line_count / sf;

        //printf("Sizes: %d, %d\n", tsx, tsy);
        
        // store data used to build the small image pixmap
        // we will calculate the stats on this subset
        float *fdata = MALLOC(sizeof(float)*tsx*tsy);

        double avg = 0.0;
        double stddev = 0.0;

        // split out the case where we have no ignore value --
        // should be quite a bit faster...
        if (meta_is_valid_double(meta->general->no_data)) {
            // Compute stats -- ignore "no data" value
            int n=0;
            for ( ii = 0 ; ii < tsy ; ii++ ) {
                for ( jj = 0 ; jj < tsx ; jj++ ) {
                    float v = get_pixel(ii*sf, jj*sf);
                    fdata[jj+ii*tsx] = v;
                    if (v != meta->general->no_data) {
                        avg += v;
                        ++n;
                    }
                }
            }
            avg /= (double)n;
            for ( ii = 0 ; ii < tsy ; ii++ ) {
                for ( jj = 0 ; jj < tsx ; jj++ ) {
                    float v = fdata[jj+ii*tsx];//get_pixel(ii*sf, jj*sf);
                    if (v != meta->general->no_data)
                        stddev += (v - avg) * (v - avg);
                }
            }
            stddev = sqrt(stddev / (double)(tsx*tsy));
        } else {
            // Compute stats -- no ignore
            for ( ii = 0 ; ii < tsy ; ii++ ) {
                for ( jj = 0 ; jj < tsx ; jj++ ) {
                    float v = get_pixel(ii*sf, jj*sf);
                    fdata[jj+ii*tsx] = v;
                    avg += v;
                }
            }
            avg /= (double)(tsx*tsy);
            for ( ii = 0 ; ii < tsy ; ii++ ) {
                for ( jj = 0 ; jj < tsx ; jj++ ) {
                    float v = fdata[jj+ii*tsx];
                    stddev += (v - avg) * (v - avg);
                }
            }
            stddev = sqrt(stddev / (double)(tsx*tsy));
        }

        //printf("Avg, StdDev: %f, %f\n", avg, stddev);

        // Set the limits of the scaling - 2-sigma on either side of the mean
        // These are globals, we will use them in the big image, too.
        g_min = avg - 2*stddev;
        g_max = avg + 2*stddev;

        unsigned char *bdata = MALLOC(sizeof(unsigned char)*tsx*tsy*3);

        // Now actually scale the data, and convert to bytes.
        // Note that we need 3 values, one for each of the RGB channels.
        printf("Building preview image...\n");
        int have_no_data = meta_is_valid_double(meta->general->no_data);
        for ( ii = 0 ; ii < tsy ; ii++ ) {
            for ( jj = 0 ; jj < tsx ; jj++ ) {
                int index = jj+ii*tsx;
                float val = fdata[index];

                unsigned char uval;
                if (have_no_data && val == meta->general->no_data)
                    uval = 0;
                else if (val < g_min)
                    uval = 0;
                else if (val > g_max)
                    uval = 255;
                else
                    uval = (unsigned char)(((val-g_min)/(g_max-g_min))*255+0.5);
            
                int n = 3*index;
                bdata[n] = uval;
                bdata[n+1] = uval;
                bdata[n+2] = uval;
            }
            asfPercentMeter((double)ii/(tsy-1));
        }

        // done with our subset
        free(fdata);

        // Create the pixbuf
        GdkPixbuf *pb =
            gdk_pixbuf_new_from_data(bdata, GDK_COLORSPACE_RGB, FALSE, 
                                     8, tsx, tsy, tsx*3, destroy_pb_data, NULL);
        
        if (!pb)
            asfPrintError("Failed to create the small pixbuf.\n");

        // Scale down to the size we actually want, using the built-in Gdk
        // scaling method, much nicer than what we did above

        // Must ensure we scale the same in each direction
        double scale_y = (double)tsy / size;
        double scale_x = (double)tsx / size;
        double scale = scale_y > scale_x ? scale_y : scale_x;
        int x_dim = tsx / scale;
        int y_dim = tsy / scale;

        printf("Scaling to %dx%d\n", x_dim, y_dim);

        pixbuf_small =
            gdk_pixbuf_scale_simple(pb, x_dim, y_dim, GDK_INTERP_BILINEAR);
        gdk_pixbuf_unref(pb);
    
        if (!pixbuf_small)
            asfPrintError("Failed to allocate scaled thumbnail pixbuf\n");
    }

    GdkPixbuf *pb2 = gdk_pixbuf_copy(pixbuf_small);
    put_bounding_box(pb2);
    return pb2;
}

void fill_small_force_reload()
{
    GdkPixbuf *pb = make_small_image(256, TRUE);
    GtkWidget *img = get_widget_checked("small_image");
    gtk_image_set_from_pixbuf(GTK_IMAGE(img), pb);
}

void fill_small()
{
    GdkPixbuf *pb = make_small_image(256, FALSE);
    GtkWidget *img = get_widget_checked("small_image");
    gtk_image_set_from_pixbuf(GTK_IMAGE(img), pb);
}
