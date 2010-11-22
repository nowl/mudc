#include "mudc.h"

#define WORLD_CONFIG_PATH      ".mudc/world.cfg"

enum columns {
    WORLD_NAME,                 /* G_TYPE_STRING */
    WORLD_HOSTNAME,             /* G_TYPE_STRING */
    WORLD_PORT,                 /* G_TYPE_INT */

    N_COLUMNS
};

enum button_responses {
    ADD_WORLD = 1,
    REMOVE_WORLD,
    CONNECT_TO_WORLD,
    CANCEL
};

static void
read_and_display_from_config(GtkListStore *store)
{
    char *home_dir = getenv("HOME");
    int len = strlen(home_dir) + sizeof(WORLD_CONFIG_PATH) + 1;
    char *fin_name = malloc(sizeof(*fin_name) * len);
    snprintf(fin_name, len, "%s/%s", home_dir, WORLD_CONFIG_PATH);

    FILE *fin = fopen(fin_name, "r");
    
    if(!fin)
    {
        /* file doesn't exist */
        return;
    }

    char name[MAX_LINE_LEN];
    char host[MAX_LINE_LEN];
    char port[MAX_LINE_LEN];
    char *tmp_name = fgets(name, MAX_LINE_LEN, fin);
    /* remove newline */
    name[strlen(name)-1] = '\0';
    char *tmp_host = fgets(host, MAX_LINE_LEN, fin);
    /* remove newline */
    host[strlen(host)-1] = '\0';
    char *tmp_port = fgets(port, MAX_LINE_LEN, fin);
    while(tmp_name)
    {
        int port_int = strtoul(port, NULL, 10);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
                           WORLD_NAME, name,
                           WORLD_HOSTNAME, host,
                           WORLD_PORT, port_int,
                           -1);

        tmp_name = fgets(name, MAX_LINE_LEN, fin);
        /* remove newline */
        name[strlen(name)-1] = '\0';
        tmp_host = fgets(host, MAX_LINE_LEN, fin);
        /* remove newline */
        host[strlen(host)-1] = '\0';
        tmp_port = fgets(port, MAX_LINE_LEN, fin);
    }

    fclose(fin);
}

static void
write_to_config(GtkListStore *store)
{
    char *home_dir = getenv("HOME");
    int len = strlen(home_dir) + sizeof(WORLD_CONFIG_PATH) + 1;
    char *fin_name = malloc(sizeof(*fin_name) * len);
    snprintf(fin_name, len, "%s/%s", home_dir, WORLD_CONFIG_PATH);

    FILE *fout = fopen(fin_name, "w");

    GValue name_value = {0};
    GValue hostname_value = {0};
    GValue port_value = {0};
    GtkTreeIter iter;
    gboolean more = FALSE;

    more = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while(more)
    {
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 WORLD_NAME,
                                 &name_value);
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 WORLD_HOSTNAME,
                                 &hostname_value);
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 WORLD_PORT,
                                 &port_value);

        char *name = strdup(g_value_get_string(&name_value));
        char *hostname = strdup(g_value_get_string(&hostname_value));
        unsigned short port = g_value_get_int(&port_value);
        
        g_value_unset(&name_value);
        g_value_unset(&hostname_value);
        g_value_unset(&port_value);

        fputs(name, fout);
        fputc('\n', fout);
        fputs(hostname, fout);
        fputc('\n', fout);
        char port_as_string[10];
        snprintf(port_as_string, 10, "%d", port);
        fputs(port_as_string, fout);
        fputc('\n', fout);
        
        more = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
        
    fclose(fout);
}

static void
add_world_response(GtkWidget *dialog,
                   gint response_id,
                   gpointer user_data)
{
    if(response_id == ADD_WORLD)
    {
        GtkWidget **widgets = user_data;
        GtkWidget *tree = widgets[0];

        GtkWidget *name_entry = widgets[1];
        GtkWidget *hostname_entry = widgets[2];
        GtkWidget *port_entry = widgets[3];
        
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const gchar *hostname = gtk_entry_get_text(GTK_ENTRY(hostname_entry));
        const gchar *port = gtk_entry_get_text(GTK_ENTRY(port_entry));

        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

        int port_int = strtoul(port, NULL, 10);

        GtkTreeIter iter;
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
                           WORLD_NAME, name,
                           WORLD_HOSTNAME, hostname,
                           WORLD_PORT, port_int,
                           -1);

        write_to_config(GTK_LIST_STORE(model));
    }
}

static void
dialog_response(GtkWidget *dialog,
                gint response_id,
                gpointer user_data)
{
    switch(response_id)
    {
    case CONNECT_TO_WORLD:
    {
        GtkWidget *tree = user_data;
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
        GList *selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);
        
        GtkTreePath *path = selected_rows->data;
        //gint *path_ind = gtk_tree_path_get_indices(path);
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
        GtkTreeIter iter;        
        gtk_tree_model_get_iter(model, &iter, path);
        GValue hostname_value = {0};
        GValue port_value = {0};
        gtk_tree_model_get_value(model, &iter, WORLD_HOSTNAME, &hostname_value);
        gtk_tree_model_get_value(model, &iter, WORLD_PORT, &port_value);

        char *hostname = strdup(g_value_get_string(&hostname_value));
        unsigned short port = g_value_get_int(&port_value);

        g_value_unset(&hostname_value);
        g_value_unset(&port_value);

        /* save off old tab_completion file */
        tab_complete_save();

        /* set up the tab completion file */
        char *home_dir = getenv("HOME");
        int len = strlen(home_dir) + MAX_LINE_LEN + 18 + 1;
        char *tab_complete_name = malloc(sizeof(*tab_complete_name) * len);

        /* create each level of the directory structure since a -p
         * type option doesn't exist AFAIK  */
        snprintf(tab_complete_name, len, "%s/.mudc/worlds/%s", home_dir, hostname);
        mkdir(tab_complete_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        snprintf(tab_complete_name, len, "%s/.mudc/worlds/%s/tab", home_dir, hostname);
        FILE *tmp_file = fopen(tab_complete_name, "r");
        if( !tmp_file )
            tmp_file = fopen(tab_complete_name, "w");
        fclose(tmp_file);            
                
        tab_complete_set_wordlist_file(tab_complete_name);
        
        /* disconnect from current telnet connection */
        telnet_close(MUDC.telnet);
        MUDC.telnet = NULL;

        /* clear text buffer */
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(MUDC.widgets.text_buffer, &start);
        gtk_text_buffer_get_end_iter(MUDC.widgets.text_buffer, &end);
        gtk_text_buffer_delete(MUDC.widgets.text_buffer, &start, &end);
        
        /* fire up the telnet connection */
        MUDC.telnet = telnet_connect(hostname, port);
        
        if(MUDC.telnet == NULL)
        {
            GtkWidget * dialog2 = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_CLOSE,
                                                        "Error connecting to %s:%d",
                                                        hostname, port);
            gtk_dialog_run(GTK_DIALOG(dialog2));
            gtk_widget_destroy(dialog2);
        }

        free(hostname);

        gtk_widget_destroy(dialog);

        break;
    }
    case ADD_WORLD:
    {
        GtkWidget *dialog2 = gtk_dialog_new_with_buttons("Add World",
                                                         GTK_WINDOW(MUDC.widgets.main_window),
                                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                         "Add", ADD_WORLD,
                                                         "Cancel", CANCEL,
                                                         NULL);

        gtk_window_set_default_size(GTK_WINDOW(dialog2), 320, 200);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog2));

        GtkWidget *ts1 = gtk_table_new(3, 2, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(ts1), 10);
        gtk_table_set_col_spacings(GTK_TABLE(ts1), 10);
        gtk_box_pack_start(GTK_BOX(content_area), ts1, TRUE, FALSE, 0);

        gtk_table_attach_defaults(GTK_TABLE(ts1), gtk_label_new("World Name"), 0, 1, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(ts1), gtk_label_new("Hostname"), 0, 1, 1, 2);
        gtk_table_attach_defaults(GTK_TABLE(ts1), gtk_label_new("Port"), 0, 1, 2, 3);

        GtkWidget *name_entry = gtk_entry_new();
        gtk_table_attach_defaults(GTK_TABLE(ts1), name_entry, 1, 2, 0, 1);
        GtkWidget *hostname_entry = gtk_entry_new();
        gtk_table_attach_defaults(GTK_TABLE(ts1), hostname_entry, 1, 2, 1, 2);
        GtkWidget *port_entry = gtk_entry_new();
        gtk_table_attach_defaults(GTK_TABLE(ts1), port_entry, 1, 2, 2, 3);
        
        GtkWidget *widgets[] = {user_data, name_entry, hostname_entry, port_entry};

        g_signal_connect(dialog2, "response",
                         G_CALLBACK(add_world_response),
                         widgets);

        gtk_widget_show_all(dialog2);
        gtk_dialog_run(GTK_DIALOG(dialog2));
        gtk_widget_destroy(dialog2);
        
        break;
    }
    case REMOVE_WORLD:
    {
        GtkWidget *tree = user_data;
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
        GList *selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);
        
        GtkTreePath *path = selected_rows->data;
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
        GtkTreeIter iter;        
        gtk_tree_model_get_iter(model, &iter, path);
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        write_to_config(GTK_LIST_STORE(model));

        break;
    }
    };
}

void
worlds_configure_run()
{
    GtkWidget *dialog= gtk_dialog_new_with_buttons("Worlds Configuration",
                                                   GTK_WINDOW(MUDC.widgets.main_window),
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   "Add World", ADD_WORLD,
                                                   "Remove World", REMOVE_WORLD,
                                                   "Connect", CONNECT_TO_WORLD,
                                                   NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 480);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        
    GtkWidget *sizer = gtk_vbox_new(FALSE, 5);

    GtkListStore *store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    
    read_and_display_from_config(store);

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start_defaults(GTK_BOX(sizer), tree);
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("World Name", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
            
    GtkWidget *button_sizer = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(sizer), button_sizer, FALSE, FALSE, 0);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(dialog_response),
                     tree);
    
    gtk_box_pack_start_defaults(GTK_BOX(content_area), sizer);
    gtk_widget_show_all(dialog);
        
    gtk_dialog_run(GTK_DIALOG(dialog));
}
