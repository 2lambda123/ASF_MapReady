#include "asf_convert_gui.h"

static gint
popup_handler(GtkWidget *widget, GdkEvent *event)
{
  GtkMenu *menu;
  GdkEventButton *event_button;
  GtkTreeSelection *selection;
  GtkWidget *files_list;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_MENU(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  files_list = glade_xml_get_widget(glade_xml, "files_list");
  event_button = (GdkEventButton *) event;
  menu = GTK_MENU(widget);
    
  if (event->type == GDK_BUTTON_PRESS && event_button->button == 3)
  {
    /* if an item is not selected in the file grid,
       select what was clicked on */

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(files_list));
    
    if (gtk_tree_selection_count_selected_rows(selection) <= 1)
    {
      GtkTreePath *path;

      if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(files_list),
					event_button->x, event_button->y,
					&path, NULL, NULL, NULL))
      {
	gtk_tree_selection_unselect_all(selection);
	gtk_tree_selection_select_path(selection, path);
	gtk_tree_path_free(path);
      }
      else
      {
	/* nothing selected, and nothing was under mouse when clicked */
	return FALSE;
      }
    }

    gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
		   event_button->button, event_button->time);

    return TRUE;
  }

  return FALSE;
}

static SIGNAL_CALLBACK gint
popup_menu_remove(GtkWidget *widget, GdkEvent *event)
{
  GtkWidget *files_list;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  /* currently support only single-select, will have to redo this
     if we ever allow multi... */

  files_list = glade_xml_get_widget(glade_xml, "files_list");
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(files_list));
  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
  }

  return TRUE;
}

static SIGNAL_CALLBACK gint
popup_menu_process(GtkWidget *widget, GdkEvent *event)
{
  GtkWidget *files_list;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  Settings *user_settings;

  /* gui should prevent this from happening */
  if (processing)
    return TRUE;

  user_settings = settings_get_from_gui();
  processing = TRUE;

  files_list = glade_xml_get_widget(glade_xml, "files_list");
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(files_list));
  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    process_item(&iter, user_settings);
  }

  processing = FALSE;
  return TRUE;
}

static SIGNAL_CALLBACK gint
popup_menu_rename(GtkWidget *widget, GdkEvent *event)
{
  GtkWidget *files_list;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;

  files_list = glade_xml_get_widget(glade_xml, "files_list");
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(files_list));

  if (gtk_tree_selection_get_selected(selection, &model, &iter))
  {
    gchar *current_output_name;
    gchar *name_without_path;

    GtkWidget *change_output_name_dialog, 
      *label_current_output_filename,
      *entry_new_output_filename;

    change_output_name_dialog =
      glade_xml_get_widget(glade_xml, "change_output_name_dialog");

    label_current_output_filename =
      glade_xml_get_widget(glade_xml, "label_current_output_filename");

    entry_new_output_filename =
      glade_xml_get_widget(glade_xml, "entry_new_output_filename");

    gtk_tree_model_get(model, &iter, 1, &current_output_name, -1);
    name_without_path = g_path_get_basename(current_output_name);

    gtk_label_set_text(GTK_LABEL(label_current_output_filename),
		       name_without_path);

    gtk_entry_set_text(GTK_ENTRY(entry_new_output_filename),
		       name_without_path);

    gtk_widget_grab_focus(entry_new_output_filename);

    free(name_without_path);
    gtk_widget_show(change_output_name_dialog);
  }
  
  return TRUE;
}

void
setup_popup_menu()
{
  GtkWidget *menu, *widget, *item;

  /* if they right click in the files list, we'll pop up */
  widget = glade_xml_get_widget(glade_xml, "files_list");

  menu = gtk_menu_new();

  item = gtk_menu_item_new_with_label("Remove");  
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );
  g_signal_connect_swapped(G_OBJECT(item), "activate",
			   G_CALLBACK(popup_menu_remove), NULL);
  gtk_widget_show(item);

  item = gtk_menu_item_new_with_label("Process");
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );  
  g_signal_connect_swapped(G_OBJECT(item), "activate",
			   G_CALLBACK(popup_menu_process), NULL);
  gtk_widget_show(item);

  item = gtk_menu_item_new_with_label("Rename Output");
  gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );  
  g_signal_connect_swapped(G_OBJECT(item), "activate",
			   G_CALLBACK(popup_menu_rename), NULL);
  gtk_widget_show(item);
  gtk_widget_show(menu);

  g_signal_connect_swapped(widget, "button_press_event",
			   G_CALLBACK(popup_handler), menu);
  g_signal_connect_swapped(widget, "popup_menu",
			   G_CALLBACK(popup_handler), menu);
}
