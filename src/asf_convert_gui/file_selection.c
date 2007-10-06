#ifdef win32

/* ugly hack here... windef.h and asf_meta.h both define a BYTE symbol. */
/* since we don't use the BYTE from asf_meta.h here, we'll #define BYTE */
/* to something else during the processing of that header, leaving BYTE */
/* defined in windef.h alone (that's the one we want)                   */

#define BYTE __byte
#include "asf.h"
#include "asf_meta.h"
#undef BYTE
#include <windows.h>
#undef DIR_SEPARATOR
#endif

#include "asf_convert_gui.h"

#ifdef USE_GTK_FILE_CHOOSER

/* If the GtkFileChooser is available -- we'll use that instead of
   GtkFileSelection
*/
static GtkWidget *browse_widget = NULL;

// called when "cancel" clicked on the GtkFileChooser
static SIGNAL_CALLBACK void cancel_clicked()
{
    gtk_widget_hide(browse_widget);
}

// called when "ok" clicked on the GtkFileChooser
static SIGNAL_CALLBACK void ok_clicked()
{
    GSList *files = gtk_file_chooser_get_filenames(
        GTK_FILE_CHOOSER(browse_widget));

    gtk_widget_hide(browse_widget);
    if (files)
    {
        GSList *iter = files;

        do {
          gchar *s = (gchar *) iter->data;
          add_to_files_list(s);
          g_free(s);
          iter =  iter->next;
        }
        while(iter);

        g_slist_free(files);
        show_queued_thumbnails();
    }
}

// sets up the file chooser dialog
static void create_file_chooser_dialog()
{
    GtkWidget *parent = get_widget_checked("asf_convert");

    browse_widget = gtk_file_chooser_dialog_new(
        "Open Image File", GTK_WINDOW(parent),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, //Cancel button
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,   //Open button
        NULL);

    // we need to extract the buttons, so we can connect them to our
    // button handlers, above
    GtkHButtonBox *box =
        (GtkHButtonBox*)(((GtkDialog*)browse_widget)->action_area);
    GList *buttons = box->button_box.box.children;

    GtkWidget *cancel_btn = ((GtkBoxChild*)buttons->data)->widget;
    GtkWidget *ok_btn = ((GtkBoxChild*)buttons->next->data)->widget;

    g_signal_connect((gpointer)cancel_btn, "clicked",
        G_CALLBACK(cancel_clicked), NULL);
    g_signal_connect((gpointer)ok_btn, "clicked",
        G_CALLBACK(ok_clicked), NULL);

    // add the filters
    GtkFileFilter *D_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(D_filt, "CEOS Data Files (*.D)");
    gtk_file_filter_add_pattern(D_filt, "*.D");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), D_filt);

    GtkFileFilter *stf_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(stf_filt, "STF Files (*.000)");
    gtk_file_filter_add_pattern(stf_filt, "*.000");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), stf_filt);

    GtkFileFilter *raw_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(raw_filt, "RAW Files (*.raw)");
    gtk_file_filter_add_pattern(raw_filt, "*.raw");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), raw_filt);

    GtkFileFilter *geotiff_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(geotiff_filt, "GeoTIFF Files (*.tif)");
    gtk_file_filter_add_pattern(geotiff_filt, "*.tif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), geotiff_filt);

    GtkFileFilter *cpx_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(cpx_filt, "Complex Files (*.cpx)");
    gtk_file_filter_add_pattern(cpx_filt, "*.cpx");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), cpx_filt);

    GtkFileFilter *alos_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(alos_filt, "ALOS Leader Files (LED-*)");
    gtk_file_filter_add_pattern(alos_filt, "LED-*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), alos_filt);

    GtkFileFilter *airsar_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(airsar_filt, "AirSAR Leader Files (*.airsar)");
    gtk_file_filter_add_pattern(airsar_filt, "*.airsar");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), airsar_filt);

    GtkFileFilter *img_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(img_filt, "ASF Internal Files (*.img)");
    gtk_file_filter_add_pattern(img_filt, "*.img");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), img_filt);

    GtkFileFilter *all_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filt, "All Files (*.*)");
    gtk_file_filter_add_pattern(all_filt, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), all_filt);

    // allow multi-select
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(browse_widget), TRUE);

    // we need to make these modal -- if the user opens multiple "open"
    // dialogs, we'll get confused on the callbacks
    gtk_window_set_modal(GTK_WINDOW(browse_widget), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(browse_widget), TRUE);
    gtk_dialog_set_default_response(GTK_DIALOG(browse_widget),
                                    GTK_RESPONSE_OK);
}

#endif // #ifdef USE_GTK_FILE_CHOOSER

SIGNAL_CALLBACK void
on_browse_input_files_button_clicked(GtkWidget *widget)
{
#ifdef win32
    OPENFILENAME of;
    int retval;
    char fname[1024];

    fname[0] = '\0';

    memset(&of, 0, sizeof(of));

#ifdef OPENFILENAME_SIZE_VERSION_400
    of.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    of.lStructSize = sizeof(of);
#endif

    of.hwndOwner = NULL;
    of.lpstrFilter = "CEOS Level 1 Data Files (*.D)\0*.D\0"
        "CEOS Level 0 Data Files (*.raw)\0*.raw\0"
        "STF Files (*.000)\0*.000\0"
        "GeoTIFF Files (*.tif)\0*.tif\0"
        "Complex Files (*.cpx)\0*.cpx\0"
        "ALOS Files (LED-*)\0LED-*\0"
        "AirSAR Files (*.airsar)\0*.airsar\0"
        "ASF Internal Files (*.img)\0*.img\0"
        "All Files\0*\0";
    of.lpstrCustomFilter = NULL;
    of.nFilterIndex = 1;
    of.lpstrFile = fname;
    of.nMaxFile = sizeof(fname);
    of.lpstrFileTitle = NULL;
    of.lpstrInitialDir = ".";
    of.lpstrTitle = "Select File";
    of.lpstrDefExt = NULL;
    of.Flags = OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    retval = GetOpenFileName(&of);

    if (!retval) {
        if (CommDlgExtendedError())
            message_box("File dialog box error");
        return;
    }

    /* the returned "fname" has the following form:            */
    /*   <directory>\0<first file>\0<second file>\0<third ...  */
    char * dir = strdup(fname);
    char * p = fname + strlen(dir) + 1;

    if (*p) {
        while (*p) {
            char * dir_and_file =
                malloc(sizeof(char)*(strlen(dir)+strlen(p)+5));
            sprintf(dir_and_file, "%s%c%s", dir, DIR_SEPARATOR, p);
            add_to_files_list(dir_and_file);
            p += strlen(p) + 1;
            free(dir_and_file);
        }
    } else {
        add_to_files_list(dir);
    }

    free(dir);
  show_queued_thumbnails();

#else // #ifdef win32

    /* Linux version -- use GtkFileChooser if possible */

#ifdef USE_GTK_FILE_CHOOSER

    if (!browse_widget)
        create_file_chooser_dialog();

    gtk_widget_show(browse_widget);

#else // #ifdef USE_GTK_FILE_CHOOSER

    GtkWidget *file_selection_dialog =
        get_widget_checked("input_file_selection");

    gtk_widget_show(file_selection_dialog);

#endif // #ifdef USE_GTK_FILE_CHOOSER
#endif // #ifdef win32
}

void
hide_input_file_selection_dialog()
{
    GtkWidget *file_selection_dialog =
        get_widget_checked("input_file_selection");

    gtk_widget_hide(file_selection_dialog);
}

SIGNAL_CALLBACK void
on_input_file_selection_cancel_button_clicked(GtkWidget *widget)
{
    hide_input_file_selection_dialog();
}

SIGNAL_CALLBACK gboolean
on_input_file_selection_delete_event(GtkWidget *w)
{
    hide_input_file_selection_dialog();
    return TRUE;
}

SIGNAL_CALLBACK gboolean
on_input_file_selection_destroy_event(GtkWidget *w)
{
    hide_input_file_selection_dialog();
    return TRUE;
}

SIGNAL_CALLBACK gboolean
on_input_file_selection_destroy(GtkWidget *w)
{
    hide_input_file_selection_dialog();
    return TRUE;
}

SIGNAL_CALLBACK void
on_input_file_selection_ok_button_clicked(GtkWidget *widget)
{
    GtkWidget *file_selection_dialog;
    gchar **selections;
    gchar **current;
    int i, n;

    file_selection_dialog =
        get_widget_checked("input_file_selection");

    selections = gtk_file_selection_get_selections(
        GTK_FILE_SELECTION(file_selection_dialog));

    current = selections;
    i = n = 0;

    while (*current)
    {
        /* second clause here allows silent fail for .L files, PR 92 */
        if (add_to_files_list(*current) || is_L_file(*current))
            ++i;

        ++current;
        ++n;
    }

    if (i != n)
    {
        if (n == 1 || i == 0)
        {
            message_box("Error: Unrecognized file type or file extension.\n"
                "Please select the leader (.L, LED-, etc) file.\n");
        }
        else
        {
            message_box("Some of the files were not added -- unknown types or extensions.");
        }
    }

    g_strfreev(selections);
    gtk_widget_hide(file_selection_dialog);

  show_queued_thumbnails();
}
