#include "mudc.h"

#define MAIN_WINDOW_TITLE                "Mudc v. 0.1"
#define UPDATE_INTERVAL_MS               (0.1 * 1000)
#define UPDATE_INTERVAL_SECS             1
#define MAX_NUM_TEXT_BUFFER_LINES        5000

void
view_font_set(GtkTextView *text_view, char *font_name)
{
    PangoFontDescription * pfd = pango_font_description_from_string(font_name);
    gtk_widget_modify_font(text_view, pfd);
    pango_font_description_free(pfd);
}

static gboolean
telnet_processing_callback(gpointer data)
{
    /* TODO: possibly return FALSE here and then add this back into the look */
    if(!data)
        return TRUE;

    int new_data = telnet_process((struct telnetp *)data);
    
    if(new_data) {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(MUDC.widgets.text_buffer, &iter);

        /* TODO: add everything to tab completion */
/*
        GtkTextIter mark;
        gtk_text_buffer_get_iter_at_mark(MUDC.widgets.text_buffer, &mark, MUDC.widgets.text_mark);
        gchar *text = gtk_text_buffer_get_text(MUDC.widgets.text_buffer, &mark, &iter, FALSE);
        printf("adding %s\n", text);
        tab_complete_add_sentence(text);
        g_free(text);
*/

        gtk_text_buffer_move_mark(MUDC.widgets.text_buffer, MUDC.widgets.text_mark, &iter);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(MUDC.widgets.text_view), MUDC.widgets.text_mark);

        /* clear old lines if we've hit the max number of lines */
        int num_lines = gtk_text_buffer_get_line_count(MUDC.widgets.text_buffer);
        if(num_lines > MAX_NUM_TEXT_BUFFER_LINES)
        {
            GtkTextIter start, end;
            gtk_text_buffer_get_start_iter(MUDC.widgets.text_buffer, &start);
            gtk_text_buffer_get_iter_at_line(MUDC.widgets.text_buffer, &end, num_lines - MAX_NUM_TEXT_BUFFER_LINES);
            gtk_text_buffer_delete(MUDC.widgets.text_buffer, &start, &end);
        }
    }        

    return TRUE;
}

static void
initialize_from_config()
{
    char *font_name = config_get(CONFIG_MAIN_WINDOW_FONT);
    if(font_name)
        view_font_set(GTK_TEXT_VIEW(MUDC.widgets.text_view), font_name);
    font_name = config_get(CONFIG_TEXT_ENTRY_FONT);
    if(font_name)
        view_font_set(GTK_TEXT_VIEW(MUDC.widgets.entry_view), font_name);
}

void 
gui_init(int *argc, char **argv[])
{
    GtkWidget *sizer_main;
    GtkWidget *menu_bar;
    
    gtk_init(argc, argv);
    
//    g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, UPDATE_INTERVAL_SECS,
//                               telnet_processing_callback, MUDC.telnet, NULL);
    g_timeout_add_full(G_PRIORITY_DEFAULT, UPDATE_INTERVAL_MS,
                       telnet_processing_callback, MUDC.telnet, NULL);
    
    /* set up main window */
    MUDC.widgets.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(MUDC.widgets.main_window), (gchar *)MAIN_WINDOW_TITLE);
    
    
    gtk_window_set_default_size(GTK_WINDOW(MUDC.widgets.main_window), 1024, 768);
    
    /* set up main window sizer */
    sizer_main = gtk_vbox_new(TRUE, 5);
    gtk_box_set_homogeneous(GTK_BOX(sizer_main), FALSE);
    
    g_signal_connect_swapped(MUDC.widgets.main_window, "delete-event",
                             G_CALLBACK(menu_close_program),
                             MUDC.widgets.main_window);

    /* set up menu bar */
    menu_bar = gtk_menu_bar_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

    GtkWidget *settings_menu = gtk_menu_new();
    GtkWidget *settings_item = gtk_menu_item_new_with_label("Settings");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_item), settings_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), settings_item);

    /* set up file menu */
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    g_signal_connect(quit_item, "activate",
                     G_CALLBACK(menu_handler),
                     (gpointer)"file.quit");

    /* set up settings menu */
    GtkWidget *pref_config_item = gtk_menu_item_new_with_label("Preferences...");
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), pref_config_item);
    g_signal_connect(pref_config_item, "activate",
                     G_CALLBACK(menu_handler),
                     (gpointer)"settings.preferences");
    
    gtk_box_pack_start(GTK_BOX(sizer_main), menu_bar, FALSE, FALSE, 0);

    /* set up text view */
    GtkWidget *text_view_scroll = gtk_scrolled_window_new(NULL, NULL);

    g_object_set(text_view_scroll,
                 "hscrollbar-policy",
                 GTK_POLICY_AUTOMATIC,
                 NULL);
    
    MUDC.widgets.text_view = gtk_text_view_new();
    MUDC.widgets.text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(MUDC.widgets.text_view));
    telnet_set_gtk_text_buffer(MUDC.widgets.text_buffer);
    MUDC.widgets.vert_adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(text_view_scroll));
    telnet_set_gtk_vert_adj(MUDC.widgets.vert_adj);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(MUDC.widgets.text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(MUDC.widgets.text_view), FALSE);

    MUDC.widgets.text_mark = gtk_text_mark_new("end mark", FALSE);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(MUDC.widgets.text_buffer, &iter);
    gtk_text_buffer_add_mark(MUDC.widgets.text_buffer, MUDC.widgets.text_mark, &iter);

    gtk_container_add(GTK_CONTAINER(text_view_scroll), MUDC.widgets.text_view);

    gtk_box_pack_start(GTK_BOX(sizer_main), text_view_scroll, TRUE, TRUE, 0);
    
    /* set up text entry */
    MUDC.widgets.entry_view = gtk_text_view_new();
    MUDC.widgets.entry_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(MUDC.widgets.entry_view));
    
    g_signal_connect(MUDC.widgets.entry_view, "key-press-event",
                     G_CALLBACK(entry_handler_keypress),
                     NULL);                     

    gtk_box_pack_start(GTK_BOX(sizer_main), MUDC.widgets.entry_view, FALSE, TRUE, 0);

    /* finalize window */
    gtk_container_add(GTK_CONTAINER(MUDC.widgets.main_window), sizer_main);
    
    gtk_widget_show_all(MUDC.widgets.main_window);

    /* intialize gui from config file */
    initialize_from_config();

    /* set colors */
    GdkColor color;
    gdk_color_parse("#7f7f7f", &color);
    gtk_widget_modify_text(MUDC.widgets.text_view, GTK_STATE_NORMAL, &color);
    gdk_color_parse("#000", &color);
    gtk_widget_modify_base(MUDC.widgets.text_view, GTK_STATE_NORMAL, &color);

    gtk_widget_grab_focus(MUDC.widgets.entry_view);
}
