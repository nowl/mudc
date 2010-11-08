#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <string.h>
#include <assert.h>

#include "telnet.h"
#include "config.h"

#define MAIN_WINDOW_TITLE                "Mudc v. 0.1"
#define UPDATE_INTERVAL_SECONDS          1
#define MAX_NUM_TEXT_BUFFER_LINES        5000

static struct telnetp *telnet       = NULL;
static GtkWidget      *text_view    = NULL;
static GtkTextBuffer  *text_buffer  = NULL;
static GtkWidget      *entry_view   = NULL;
static GtkTextBuffer  *entry_buffer = NULL;
static GtkAdjustment  *vert_adj     = NULL;
static GtkTextMark *text_mark = NULL;

static void close_dialog_response(GtkDialog *dialog,
                                  gint response_id,
                                  gpointer user_data)
{
    if(response_id == GTK_RESPONSE_YES)
    {
        if(telnet)
            telnet_close(telnet);
        gtk_main_quit();
    }
}

static gboolean close_program(GtkWidget *widget,
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

static char **command_history = NULL;
static size_t command_history_c = 0;
static int command_history_i = 0;
static int command_history_p = 0;

static gboolean entry_keypress(GtkWidget   *widget,
                               GdkEventKey *event,
                               gpointer     user_data) 
{
    if(event->keyval == GDK_KEY_Return)
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry_buffer, &start);
        gtk_text_buffer_get_end_iter(entry_buffer, &end);

        gchar *text = gtk_text_buffer_get_text(entry_buffer, &start, &end, FALSE);

        /* clear the text */
        gtk_text_buffer_delete(entry_buffer, &start, &end);

        /* send the text */
        telnet_send(telnet, text);

        /* save in history */
        if(command_history != NULL)
        {
            if(command_history_c == command_history_i)
                command_history = memory_grow_to_size(command_history, 
                                                      sizeof(*command_history),
                                                      &command_history_c,
                                                      command_history_c*2);
        }
        else
            command_history = memory_grow_to_size(command_history,
                                                  sizeof(*command_history),
                                                  &command_history_c,
                                                  4);
        
        command_history[command_history_i++] = strdup(text);
        command_history_p = command_history_i;
    
        g_free(text);

        return TRUE;
    }
    else if(event->keyval == GDK_KEY_Up)
    {
        command_history_p--;
        if(command_history_p < 0)
            command_history_p = 0;

        char *cmd = command_history[command_history_p];
        
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry_buffer, &start);
        gtk_text_buffer_get_end_iter(entry_buffer, &end);
        
        gtk_text_buffer_delete(entry_buffer, &start, &end);

        gtk_text_buffer_insert_at_cursor(entry_buffer, cmd, strlen(cmd));
    }
    else if(event->keyval == GDK_KEY_Down)
    {
        command_history_p++;
        if(command_history_p > command_history_i-1)
            command_history_p = command_history_i-1;

        char *cmd = command_history[command_history_p];
        
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry_buffer, &start);
        gtk_text_buffer_get_end_iter(entry_buffer, &end);
        
        gtk_text_buffer_delete(entry_buffer, &start, &end);

        gtk_text_buffer_insert_at_cursor(entry_buffer, cmd, strlen(cmd));
    }

    return FALSE;    
}

static void
font_set(char *font_name)
{
    PangoFontDescription * pfd = pango_font_description_from_string(font_name);
    gtk_widget_modify_font(text_view, pfd);
    pango_font_description_free(pfd);
}

static void font_dialog_response(GtkDialog *dialog,
                                 gint response_id,
                                 gpointer user_data)
{
    gchar *font_name = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
    if(font_name)
    {
        /* set in config */
        config_set(CONFIG_FONT_NAME, font_name);

        font_set(font_name);

        g_free(font_name);
    }
}

static void
menu_selection(GtkMenuItem *item,
               gpointer user_data)
{
    if( strcmp(user_data, "file.quit") == 0)
    {
        close_program(NULL, NULL, NULL);
    } else if( strcmp(user_data, "settings.fonts") == 0) {
        GtkWidget *font_dlg = gtk_font_selection_dialog_new("Select foreground font");
        
        /* fill in the currently used font */
        char *font_name = config_get(CONFIG_FONT_NAME);
        if(font_name)
            gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(font_dlg), font_name);

        g_signal_connect(font_dlg, "response",
                         G_CALLBACK(font_dialog_response),
                         NULL);         
        gtk_dialog_run(GTK_DIALOG(font_dlg));
        gtk_widget_destroy(font_dlg);                
    }
}              

static gboolean
telnet_processing_callback(gpointer data)
{
    int new_data = telnet_process((struct telnetp *)data);
    
    if(new_data) {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(text_buffer, &iter);
        gtk_text_buffer_move_mark(text_buffer, text_mark, &iter);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(text_view), text_mark);

        /* clear old lines if we've hit the max number of lines */
        int num_lines = gtk_text_buffer_get_line_count(text_buffer);
        if(num_lines > MAX_NUM_TEXT_BUFFER_LINES)
        {
            GtkTextIter start, end;
            gtk_text_buffer_get_start_iter(text_buffer, &start);
            gtk_text_buffer_get_iter_at_line(text_buffer, &end, num_lines - MAX_NUM_TEXT_BUFFER_LINES);
            gtk_text_buffer_delete(text_buffer, &start, &end);
        }
    }        

    return TRUE;
}

int
main(int argc, char *argv[])
{
    GtkWidget *main_window;
    GtkWidget *sizer_main;
    GtkWidget *menu_bar;

    config_read();

    //telnet = telnet_connect("oak", 23);
    //telnet = telnet_connect("realmsofdespair.com", 4000);
    telnet = telnet_connect("aardmud.org", 4000);

    if(!telnet)
    {
        printf("problem connecting\n");
        return 1;
    }

    gtk_init(&argc, &argv);
    
    g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, UPDATE_INTERVAL_SECONDS,
                               telnet_processing_callback, telnet, NULL);
    
    /* set up main window */
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), (gchar *)MAIN_WINDOW_TITLE);
    
    
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);
    
    /* set up main window sizer */
    sizer_main = gtk_vbox_new(TRUE, 5);
    gtk_box_set_homogeneous(GTK_BOX(sizer_main), FALSE);
    
    g_signal_connect_swapped(main_window, "delete-event",
                             G_CALLBACK(close_program),
                             main_window);

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
                     G_CALLBACK(menu_selection),
                     (gpointer)"file.quit");

    /* set up settings menu */
    GtkWidget *font_config_item = gtk_menu_item_new_with_label("Fonts");
    gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), font_config_item);
    g_signal_connect(font_config_item, "activate",
                     G_CALLBACK(menu_selection),
                     (gpointer)"settings.fonts");
    
    gtk_box_pack_start(GTK_BOX(sizer_main), menu_bar, FALSE, FALSE, 0);

    /* set up text view */
    GtkWidget *text_view_scroll = gtk_scrolled_window_new(NULL, NULL);

    g_object_set(text_view_scroll,
                 "hscrollbar-policy",
                 GTK_POLICY_AUTOMATIC,
                 NULL);
    
    text_view = gtk_text_view_new();
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    telnet_set_gtk_text_buffer(text_buffer);
    vert_adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(text_view_scroll));
    telnet_set_gtk_vert_adj(vert_adj);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);

    text_mark = gtk_text_mark_new("end mark", FALSE);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(text_buffer, &iter);
    gtk_text_buffer_add_mark(text_buffer, text_mark, &iter);

    gtk_container_add(GTK_CONTAINER(text_view_scroll), text_view);

    gtk_box_pack_start(GTK_BOX(sizer_main), text_view_scroll, TRUE, TRUE, 0);
    
    /* set up text entry */
    entry_view = gtk_text_view_new();
    entry_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry_view));
    
    g_signal_connect(entry_view, "key-press-event",
                     G_CALLBACK(entry_keypress),
                     NULL);                     

    gtk_box_pack_start(GTK_BOX(sizer_main), entry_view, FALSE, TRUE, 0);

    /* finalize window */
    gtk_container_add(GTK_CONTAINER(main_window), sizer_main);
    
    gtk_widget_show_all(main_window);

    /* set font */
    char *font_name = config_get(CONFIG_FONT_NAME);
    if(font_name)
        font_set(font_name);

    /* set colors */
    GdkColor color;
    gdk_color_parse("#7f7f7f", &color);
    gtk_widget_modify_text(text_view, GTK_STATE_NORMAL, &color);
    gdk_color_parse("#000", &color);
    gtk_widget_modify_base(text_view, GTK_STATE_NORMAL, &color);

    gtk_widget_grab_focus(entry_view);

    gtk_main();
    
    return 0;
}
