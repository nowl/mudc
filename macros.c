#include <gdk/gdkkeysyms.h>

#include "mudc.h"

enum {
    STATE_FROM_ADD_MACRO,
    STATE_FROM_MAIN_WINDOW
};

static int key_press_callback_state;

enum {
    ADD_MACRO = 1,
    REMOVE_MACRO,
    RESPONSE_KEY
};

enum columns {
    MACRO_KEY,                  /* G_TYPE_STRING */
    MACRO_TEXT,                 /* G_TYPE_STRING */
    MACRO_KEYVAL,               /* G_TYPE_INT */
    MACRO_KEYMODS,              /* G_TYPE_INT */

    N_COLUMNS
};

static char *macros_filename = NULL;
static GtkWidget *tree = NULL;
static GtkWidget *macros_dialog = NULL;
static GtkListStore *store = NULL;
static gulong signal_handler_press, signal_handler_release;

/* textual names for the accepted macro keys */
static struct
{
    guint keyval;
    char *keystring;
} keyval_mapping[] = {
    {GDK_KEY_F1, "F1"},
    {GDK_KEY_F2, "F2"},
    {GDK_KEY_F3, "F3"},
    {GDK_KEY_F4, "F4"},
    {GDK_KEY_F5, "F5"},
    {GDK_KEY_F6, "F6"},
    {GDK_KEY_F7, "F7"},
    {GDK_KEY_F8, "F8"},
    {GDK_KEY_F9, "F9"},
    {GDK_KEY_F10, "F10"},
    {GDK_KEY_F11, "F11"},
    {GDK_KEY_F12, "F12"},
    {GDK_KEY_KP_Enter, "Keypad Enter"},
    {GDK_KEY_KP_Multiply, "Keypad Multiply"},
    {GDK_KEY_KP_Add, "Keypad Add"},
    {GDK_KEY_KP_Subtract, "Keypad Subtract"},
    {GDK_KEY_KP_Decimal, "Keypad Decimal"},
    {GDK_KEY_KP_Divide, "Keypad Divide"},
    {GDK_KEY_KP_0, "Keypad 0"},
    {GDK_KEY_KP_1, "Keypad 1"},
    {GDK_KEY_KP_2, "Keypad 2"},
    {GDK_KEY_KP_3, "Keypad 3"},
    {GDK_KEY_KP_4, "Keypad 4"},
    {GDK_KEY_KP_5, "Keypad 5"},
    {GDK_KEY_KP_6, "Keypad 6"},
    {GDK_KEY_KP_7, "Keypad 7"},
    {GDK_KEY_KP_8, "Keypad 8"},
    {GDK_KEY_KP_9, "Keypad 9"}, 
};

static struct
{
    
#define MOD_ALT   1
#define MOD_SHIFT 2
#define MOD_CTRL  4
    int mods;

    int key;    
} keystate;

static int keyval_mapping_len = sizeof(keyval_mapping)/sizeof(keyval_mapping[0]);

static char *
keyval_lookup()
{
    char *key = NULL;

    int i;
    for(i=0; i<keyval_mapping_len; i++)
    {
        if(keyval_mapping[i].keyval == keystate.key)
        {
            key = keyval_mapping[i].keystring;
            break;
        }
    }

    if(!key)
        return NULL;
    
    char result[128];
    i = 0;
    if((keystate.mods & MOD_ALT) != 0)
        i += snprintf(&result[i], 128-i, ((i == 0) ? "Alt" : " - Alt"));
    if((keystate.mods & MOD_CTRL) != 0)
        i += snprintf(&result[i], 128-i, ((i == 0) ? "Control" : " - Control"));
    if((keystate.mods & MOD_SHIFT) != 0)
        i += snprintf(&result[i], 128-i, ((i == 0) ? "Shift" : " - Shift"));

    i += snprintf(&result[i], 128-i, ((i != 0) ? " - %s" : "%s"), key);

    result[i] = '\0';
    return strdup(result);
}

static void
update_keystate(GdkEventKey *event)
{
    keystate.key = -1;

    if(event->type == GDK_KEY_PRESS)
    {
        switch(event->keyval)
        {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            keystate.mods |= MOD_ALT;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            keystate.mods |= MOD_SHIFT;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            keystate.mods |= MOD_CTRL;
            break;
        case GDK_KEY_F1:
        case GDK_KEY_F2:
        case GDK_KEY_F3:
        case GDK_KEY_F4:
        case GDK_KEY_F5:
        case GDK_KEY_F6:
        case GDK_KEY_F7:
        case GDK_KEY_F8:
        case GDK_KEY_F9:
        case GDK_KEY_F10:
        case GDK_KEY_F11:
        case GDK_KEY_F12:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_KP_Multiply:
        case GDK_KEY_KP_Add:
        case GDK_KEY_KP_Subtract:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Divide:
        case GDK_KEY_KP_0:
        case GDK_KEY_KP_1:
        case GDK_KEY_KP_2:
        case GDK_KEY_KP_3:
        case GDK_KEY_KP_4:
        case GDK_KEY_KP_5:
        case GDK_KEY_KP_6:
        case GDK_KEY_KP_7:
        case GDK_KEY_KP_8:
        case GDK_KEY_KP_9:
            keystate.key = event->keyval;
            break;
        default:
            break;
        }
    }
    else if(event->type == GDK_KEY_RELEASE)
    {
        switch(event->keyval)
        {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            keystate.mods &= ~MOD_ALT;
            break;
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            keystate.mods &= ~MOD_SHIFT;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            keystate.mods &= ~MOD_CTRL;
            break;
        default:
            break;
        }
    }
}

static gboolean
main_window_key_press(GtkWidget *widget,
                      GdkEvent  *event,
                      gpointer   user_data)
{
    if(event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE)
    {
        update_keystate((GdkEventKey *)event);

        if(keystate.key != -1)
        {
            /* valid macro key pressed, now look it up to see if there
             * is a macro associated with this key */
            
            GValue text_value = {0};
            GValue keyval_value = {0};
            GValue keymods_value = {0};
            GtkTreeIter iter;
            gboolean more = FALSE;
            
            more = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
            while(more)
            {
                gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter, MACRO_TEXT, &text_value);
                gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter, MACRO_KEYVAL, &keyval_value);
                gtk_tree_model_get_value(GTK_TREE_MODEL(store), &iter, MACRO_KEYMODS, &keymods_value);

                char *text = strdup(g_value_get_string(&text_value));
                int keyval = g_value_get_int(&keyval_value);
                int keymods = g_value_get_int(&keymods_value);
        
                g_value_unset(&text_value);
                g_value_unset(&keyval_value);
                g_value_unset(&keymods_value);

                if(keyval == keystate.key && keymods == keystate.mods)
                {
                    telnet_send(MUDC.telnet, text);
                    free(text);
                    return TRUE;
                }

                free(text);

                more = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
            }
        }
    }

    return FALSE;
}

void
macros_delete(guint key, GdkModifierType mods)
{
}

void
macros_load(char *filename)
{
    if(macros_filename)
        free(macros_filename);

    macros_filename = filename;

    gtk_widget_set_sensitive(MUDC.widgets.macros_menu_item, TRUE);
}

static void
read_and_display_from_config()
{

    FILE *fin = fopen(macros_filename, "r");
    
    if(!fin)
    {
        /* file doesn't exist */
        return;
    }

    char key[MAX_LINE_LEN];
    char text[MAX_LINE_LEN];
    char keyval[MAX_LINE_LEN];
    char keymods[MAX_LINE_LEN];
    
    char *tmp_key = fgets(key, MAX_LINE_LEN, fin);
    key[strlen(key)-1] = '\0';
    char *tmp_text = fgets(text, MAX_LINE_LEN, fin);
    text[strlen(text)-1] = '\0';
    char *tmp_keyval = fgets(keyval, MAX_LINE_LEN, fin);
    char *tmp_keymods = fgets(keymods, MAX_LINE_LEN, fin);
    while(tmp_text)
    {
        int keyval_int = strtoul(keyval, NULL, 10);
        int keymods_int = strtoul(keymods, NULL, 10);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           MACRO_KEY, key,
                           MACRO_TEXT, text,
                           MACRO_KEYVAL, keyval_int,
                           MACRO_KEYMODS, keymods_int,
                           -1);

        tmp_key = fgets(key, MAX_LINE_LEN, fin);
        key[strlen(key)-1] = '\0';
        tmp_text = fgets(text, MAX_LINE_LEN, fin);
        text[strlen(text)-1] = '\0';
        tmp_keyval = fgets(keyval, MAX_LINE_LEN, fin);
        tmp_keymods = fgets(keymods, MAX_LINE_LEN, fin);
    }

    fclose(fin);
}


static void
write_to_config()
{
    FILE *fout = fopen(macros_filename, "w");

    GValue key_value = {0};
    GValue text_value = {0};
    GValue keyval_value = {0};
    GValue keymods_value = {0};
    GtkTreeIter iter;
    gboolean more = FALSE;

    more = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while(more)
    {
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 MACRO_KEY,
                                 &key_value);
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 MACRO_TEXT,
                                 &text_value);
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 MACRO_KEYVAL,
                                 &keyval_value);
        gtk_tree_model_get_value(GTK_TREE_MODEL(store),
                                 &iter,
                                 MACRO_KEYMODS,
                                 &keymods_value);

        char *key = strdup(g_value_get_string(&key_value));
        char *text = strdup(g_value_get_string(&text_value));
        int keyval = g_value_get_int(&keyval_value);
        int keymods = g_value_get_int(&keymods_value);
        
        g_value_unset(&key_value);
        g_value_unset(&text_value);
        g_value_unset(&keyval_value);
        g_value_unset(&keymods_value);

        fputs(key, fout);
        fputc('\n', fout);
        fputs(text, fout);
        fputc('\n', fout);
        char as_string[64];
        snprintf(as_string, 64, "%d", keyval);
        fputs(as_string, fout);
        fputc('\n', fout);
        snprintf(as_string, 64, "%d", keymods);
        fputs(as_string, fout);
        fputc('\n', fout);

        free(key);
        free(text);
        
        more = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
        
    fclose(fout);
}

static void
macro_text_response(GtkWidget *dialog,
                    gint response_id,
                    gpointer user_data)
{
    switch(response_id)
    {
    case GTK_RESPONSE_REJECT:
        gtk_widget_destroy(dialog);
        break;
    case GTK_RESPONSE_ACCEPT:
    {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(user_data));
        char *keystring = keyval_lookup();
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
                           MACRO_KEY, keystring,
                           MACRO_TEXT, text,
                           MACRO_KEYVAL, keystate.key,
                           MACRO_KEYMODS, keystate.mods,
                           -1);

        write_to_config(store);
        break;
    }
    }
}

static void
macro_key_response(GtkWidget *dialog,
                   gint response_id,
                   gpointer user_data)
{
    switch(response_id)
    {
    case GTK_RESPONSE_REJECT:
        gtk_widget_destroy(dialog);
        break;
    case RESPONSE_KEY:
    {
        GtkWidget *dlg = gtk_dialog_new_with_buttons("Define Macro Text",
                                                     GTK_WINDOW(dialog),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                                     NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
        
        GtkWidget *sizer = gtk_vbox_new(FALSE, 5);
        gtk_box_pack_start_defaults(GTK_BOX(content_area), sizer);

        GtkWidget *text_entry = gtk_entry_new();
        
        gtk_box_pack_start_defaults(GTK_BOX(sizer), text_entry);                                    
        
        g_signal_connect(dlg, "response",
                         G_CALLBACK(macro_text_response),
                         text_entry);    

        g_signal_connect_swapped(dlg, "delete-event",
                                 G_CALLBACK(gtk_widget_destroy),
                                 dlg);

        gtk_widget_show_all(dlg);

        gtk_widget_hide(dialog);

        gtk_dialog_run(GTK_DIALOG(dlg));

        gtk_widget_destroy(dialog);
        break;
    }
    };
}

static gboolean
macro_define_key_press(GtkWidget *widget,
                       GdkEvent  *event,
                       gpointer   user_data)
{
    if(event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE)
    {
        update_keystate((GdkEventKey *)event);
        macro_key_response(widget, RESPONSE_KEY, NULL);
    }

    return FALSE;
}

static void
dialog_response(GtkWidget *dialog,
                gint response_id,
                gpointer user_data)
{
    switch(response_id)
    {
    case ADD_MACRO:
    {
        GtkWidget *dlg = gtk_dialog_new_with_buttons("Define Macro Key",
                                                     GTK_WINDOW(dialog),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     GTK_STOCK_CANCEL,
                                                     GTK_RESPONSE_REJECT,
                                                     NULL);

        key_press_callback_state = STATE_FROM_ADD_MACRO;

        gtk_widget_add_events(dlg, GDK_KEY_PRESS_MASK);
        
        g_signal_connect(dlg, "key-press-event",
                         G_CALLBACK(macro_define_key_press),
                         NULL);
        g_signal_connect(dlg, "key-release-event",
                         G_CALLBACK(macro_define_key_press),
                         NULL);


        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
        
        GtkWidget *sizer = gtk_vbox_new(FALSE, 5);
        gtk_box_pack_start_defaults(GTK_BOX(content_area), sizer);

        gtk_box_pack_start_defaults(GTK_BOX(sizer),
                                    gtk_label_new("Press a valid macro key.\n\nThese include F1-F12, keypad keys, and any\n combination of alt/control/shift with these keys."));
        
        g_signal_connect(dlg, "response",
                         G_CALLBACK(macro_key_response),
                         NULL);
    

        g_signal_connect_swapped(dlg, "delete-event",
                                 G_CALLBACK(gtk_widget_destroy),
                                 dlg);

        gtk_widget_show_all(dlg);
        
        gtk_dialog_run(GTK_DIALOG(dlg));
        break;
    }
    case REMOVE_MACRO:
    {
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
        GList *selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);
        
        GtkTreePath *path = selected_rows->data;
        GtkTreeIter iter;        
        gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
        gtk_list_store_remove(store, &iter);

        write_to_config(store);
        break;
    }
    }
}

static void
macros_dialog_hide(GtkWidget *widget)
{
    g_signal_handler_unblock(MUDC.widgets.main_window, signal_handler_press);
    g_signal_handler_unblock(MUDC.widgets.main_window, signal_handler_release);
    key_press_callback_state = STATE_FROM_MAIN_WINDOW;

    gtk_widget_hide(widget);
}

void
macros_configure_run()
{
    g_signal_handler_block(MUDC.widgets.main_window, signal_handler_press);
    g_signal_handler_block(MUDC.widgets.main_window, signal_handler_release);

    GtkWidget *dialog;
    if(macros_dialog)
    {
        dialog = macros_dialog;
    }
    else
    {
        dialog = gtk_dialog_new_with_buttons("Macro Configuration",
                                             GTK_WINDOW(MUDC.widgets.main_window),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             "Add Macro", ADD_MACRO,
                                             "Remove Macro", REMOVE_MACRO,
                                             NULL);
        
        gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 480);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        
        GtkWidget *sizer = gtk_vbox_new(FALSE, 5);

        store = gtk_list_store_new(N_COLUMNS,
                                   G_TYPE_STRING, G_TYPE_STRING, 
                                   G_TYPE_INT, G_TYPE_INT);
        
        tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Macro", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Text to send to MUD", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
        
        GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
        gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
        
        read_and_display_from_config(store);
        
        gtk_box_pack_start_defaults(GTK_BOX(sizer), tree);
        
        gtk_box_pack_start_defaults(GTK_BOX(content_area), sizer);
        
        g_signal_connect(dialog, "response",
                         G_CALLBACK(dialog_response),
                         NULL);
        
        g_signal_connect_swapped(dialog, "delete-event",
                                 G_CALLBACK(macros_dialog_hide),
                                 dialog);
        
        gtk_widget_show_all(dialog);
        
        macros_dialog = dialog;
    }
    
    gtk_dialog_run(GTK_DIALOG(dialog));
}

void
macros_init()
{

    gtk_widget_add_events(MUDC.widgets.main_window, GDK_KEY_PRESS_MASK);
    
    signal_handler_press = g_signal_connect(MUDC.widgets.main_window, "key-press-event",
                                            G_CALLBACK(main_window_key_press),
                                            NULL);
    signal_handler_release = g_signal_connect(MUDC.widgets.main_window, "key-release-event",
                                              G_CALLBACK(main_window_key_press),
                                              NULL);

    key_press_callback_state = STATE_FROM_MAIN_WINDOW;
    keystate.mods = 0;
}
