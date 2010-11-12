#include "mudc.h"

static void close_dialog_response(GtkDialog *dialog,
                                  gint response_id,
                                  gpointer user_data)
{
    if(response_id == GTK_RESPONSE_YES)
    {
        if(MUDC.telnet)
            telnet_close(MUDC.telnet);
        gtk_main_quit();
    }
}

gboolean 
menu_close_program(GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data)
{
#if 0
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO,
                                               (const gchar *)"Are you sure you wish to exit?");

    g_signal_connect(dialog, "response",
                     G_CALLBACK(close_dialog_response),
                     NULL);
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
#endif

    close_dialog_response(NULL, GTK_RESPONSE_YES, NULL);

    return TRUE;
}

static void
remove_cb(GtkWidget *child, gpointer data)
{
    GtkContainer *sizer = data;

    gtk_container_remove(sizer, child);
}

static void
clear_sizer(GtkContainer *sizer)
{
    gtk_container_foreach(sizer, remove_cb, sizer);
}

struct font_callback_data
{
    char *config_type;
    GtkWidget *text_view;
    GtkWidget *button;
};

static struct font_callback_data font_cb_data_main, font_cb_data_entry;

static void
font_dialog_response(GtkDialog *dialog,
                     gint response_id,
                     gpointer data)
{
    /* TODO: make sure this is set properly */
    //if(response_id == 
    gchar *font_name = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
    if(font_name)
    {
        /* set in config */
        config_set(((struct font_callback_data *)data)->config_type, font_name);
        
        view_font_set(GTK_TEXT_VIEW(((struct font_callback_data *)data)->text_view),
                      font_name);

        GtkWidget *button = ((struct font_callback_data *)data)->button;
        gtk_button_set_label(GTK_BUTTON(button), font_name);
        
        g_free(font_name);
    }
}

static void
font_select_cb(GtkWidget *src,
               gpointer *data)
{
    GtkWidget *font_dlg = gtk_font_selection_dialog_new("Select font");
        
    /* fill in the currently used font */
    char *font_name = config_get(((struct font_callback_data *)data)->config_type);
    if(font_name)
        gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(font_dlg), font_name);
    
    g_signal_connect(font_dlg, "response",
                     G_CALLBACK(font_dialog_response),
                     data);         
    gtk_dialog_run(GTK_DIALOG(font_dlg));
    gtk_widget_destroy(font_dlg);
}

static void
tree_selection_cb(GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *selected_text;
    
    GtkWidget **cb_data = data;
    GtkWidget *sizer = cb_data[0];
    GtkWidget *dialog = cb_data[1];

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &selected_text, -1);
        
        clear_sizer(GTK_CONTAINER(sizer));

        if(strcmp(selected_text, "Fonts") == 0)
        {
            GtkWidget *s1 = gtk_hbox_new(FALSE, 5);
            gtk_box_pack_start(GTK_BOX(sizer), s1, TRUE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(s1), gtk_label_new("Main window font"), FALSE, FALSE, 10);
            char *font_name = config_get(CONFIG_MAIN_WINDOW_FONT);
            if(!font_name) font_name = "Not Set";
            GtkWidget *b1 = gtk_button_new_with_label(font_name);
            font_cb_data_main.config_type = CONFIG_MAIN_WINDOW_FONT;
            font_cb_data_main.text_view = MUDC.widgets.text_view;
            font_cb_data_main.button = b1;
            g_signal_connect(b1, "clicked",
                             G_CALLBACK(font_select_cb),
                             &font_cb_data_main);
            gtk_box_pack_start(GTK_BOX(s1), b1, FALSE, FALSE, 0);

            s1 = gtk_hbox_new(FALSE, 5);
            gtk_box_pack_start(GTK_BOX(sizer), s1, TRUE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(s1), gtk_label_new("Text entry font"), FALSE, FALSE, 10);
            font_name = config_get(CONFIG_TEXT_ENTRY_FONT);
            if(!font_name) font_name = "Not Set";
            b1 = gtk_button_new_with_label(font_name);
            font_cb_data_entry.config_type = CONFIG_TEXT_ENTRY_FONT;
            font_cb_data_entry.text_view = MUDC.widgets.entry_view;
            font_cb_data_entry.button = b1;
            g_signal_connect(b1, "clicked",
                             G_CALLBACK(font_select_cb),
                             &font_cb_data_entry);
            gtk_box_pack_start(GTK_BOX(s1), b1, FALSE, FALSE, 0);
        }
        else if(strcmp(selected_text, "Colors") == 0)
        {
            /* TODO: finish this */
        }
                
        gtk_widget_show_all(dialog);
          
        g_free (selected_text);
    }
}

void menu_handler(GtkMenuItem *item,
                  gpointer user_data)
{
    if( strcmp(user_data, "file.quit") == 0)
    {
        menu_close_program(NULL, NULL, NULL);
    } else if( strcmp(user_data, "settings.preferences") == 0) {
        GtkWidget *dialog= gtk_dialog_new_with_buttons("Mudc Preferences",
                                                       GTK_WINDOW(MUDC.widgets.main_window),
                                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                                       NULL);

        gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 480);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        
        GtkWidget *sizer = gtk_hbox_new(FALSE, 5);

        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, "Fonts", -1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, "Colors", -1);
        GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW (tree), column);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);
        
        GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree));
        gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
            
        GtkWidget * pref_sizer = gtk_vbox_new(TRUE, 5);
        GtkWidget *cb_data[] = {pref_sizer, dialog};
        g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(tree_selection_cb), cb_data);
        
        gtk_box_pack_start(GTK_BOX(sizer), tree, FALSE, FALSE, 0);
        gtk_box_pack_start_defaults(GTK_BOX(sizer), pref_sizer);
        
        gtk_box_pack_start_defaults(GTK_BOX(content_area), sizer);
        gtk_widget_show_all(dialog);
        
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}              
