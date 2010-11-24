#ifndef __MUDC_H__
#define __MUDC_H__

#include <sys/stat.h>

#include "config.h"
#include "telnet.h"
#include "config.h"
#include "tab_complete.h"
#include "macros.h"

#define MAX_LINE_LEN           512

struct mudc
{
    struct telnetp *telnet;
    
    /* gtk stuff */
    struct
    {
        GtkWidget     *main_window;
        GtkWidget     *text_view;
        GtkTextBuffer *text_buffer;
        GtkWidget     *entry_view;
        GtkTextBuffer *entry_buffer;
        GtkAdjustment *vert_adj;
        GtkTextMark   *text_mark;
    } widgets;

    struct 
    {
        char   **command_history;
        size_t   command_history_c;
        int      command_history_i;
        int      command_history_p;
        
        char   *text_to_find;
        size_t  text_to_find_c;
    } buffers;

};

extern struct mudc MUDC;

/* gtk handlers */
gboolean
entry_handler_keypress(GtkWidget   *widget,
                       GdkEventKey *event,
                       gpointer     user_data);

void settings_dialog_run();

gboolean 
menu_close_program(GtkWidget *widget,
                   GdkEvent *event,
                   gpointer data);
    
void gui_init(int *argc, char **argv[]);
void view_font_set(GtkWidget *text_view, char *font_name);
void worlds_configure_run();

#endif  /* __MUDC_H__ */
