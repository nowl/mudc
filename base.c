#include <gtk/gtk.h>

#include <string.h>
#include <assert.h>

#include "telnet.h"
#include "config.h"

#define MAIN_WINDOW_TITLE                "Mudc v. 0.1"
#define UPDATE_INTERVAL_SECONDS          1

static GtkTextView *text_view = NULL;

static void close_dialog_response(GtkDialog *dialog,
                                  gint response_id,
                                  gpointer user_data)
{
    if(response_id == GTK_RESPONSE_YES)
        gtk_main_quit();
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

static void entry_textview_size_maintainer(GtkWidget *widget,
                                           GtkRequisition *requisition,
                                           gpointer user_data)
{
	GtkRequisition req = {1, 20};
        gtk_widget_size_request(widget, &req);
}

static void view_signal(GtkTextBuffer *widget,
                        gpointer data)
{
//    GtkTextIter iter1, iter2;
    //gtk_text_buffer_get_start_iter(widget, &iter1);
    //gtk_text_buffer_get_end_iter(widget, &iter2);
//    gchar *text = gtk_text_buffer_get_text(widget, &iter1, &iter2, FALSE);
    //gtk_text_buffer_get_property("cursor-position"
//    g_print("text buffer contains: %s\n", text);
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
    telnet_process((struct telnetp *)data);

    return TRUE;
}

int
main(int argc, char *argv[])
{
    GtkWidget *main_window;
    GtkWidget *sizer_main;
    GtkWidget *entry_textview;
    GtkWidget *text_entry;
    GtkWidget *menu_bar;

    config_read();

    //struct telnetp *tn = telnet_connect("oak", 23);
    struct telnetp *tn = telnet_connect("aardmud.org", 4000);

    gtk_init(&argc, &argv);
    
    g_timeout_add_seconds(UPDATE_INTERVAL_SECONDS, telnet_processing_callback, tn);
    
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
    text_view = gtk_text_view_new();
    telnet_set_gtk_text_buffer(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)));

    gtk_box_pack_start(GTK_BOX(sizer_main), text_view, TRUE, TRUE, 0);
    
    /* set up text entry */
    text_entry = gtk_text_view_new();

    gtk_box_pack_start(GTK_BOX(sizer_main), text_entry, FALSE, TRUE, 0);

    /* finalize window */
    gtk_container_add(GTK_CONTAINER(main_window), sizer_main);
    
    gtk_widget_show_all(main_window);

    /* set font */
    char *font_name = config_get(CONFIG_FONT_NAME);
    if(font_name)
        font_set(font_name);

    gtk_main();
    
    return 0;
}
