#include <stdio.h>
#include <stdbool.h>

#include "telnet.h"

static GtkTextBuffer *text_buffer = NULL;
static GtkAdjustment *vert_adj = NULL;

static gboolean need_refresh = false;

//static char *line_buffer = NULL;
//static size_t line_buffer_c = 0;
//static int line_buffer_i = 0;

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb)
{
    text_buffer = tb;
}

void telnet_set_gtk_vert_adj(GtkAdjustment *adj)
{
    vert_adj = adj;
}

static void
telnet_callback(int type, void *data)
{
    switch(type) {
    case TC_LINE_FEED:
        //gtk_text_buffer_insert_at_cursor(text_buffer, line_buffer, line_buffer_i);
        //line_buffer_i = 0;

        gtk_text_buffer_insert_at_cursor(text_buffer, "\n", 1);
        need_refresh = true;
        break;
    case TC_CARRIAGE_RETURN:
        //gtk_text_buffer_insert_at_cursor(text_buffer, "\r", 1);
        break;
    case TC_ASCII: {
        struct ascii_callback *ac_data = data;
        char tmp[1] = {ac_data->c};
        gtk_text_buffer_insert_at_cursor(text_buffer, tmp, 1);
        //if(line_buffer_c == line_buffer_i)
        //    line_buffer = memory_grow_to_size(line_buffer, &line_buffer_c, line_buffer_i+1);
        //line_buffer[line_buffer_i++] = ac_data->c;
        break;
    }
    case TC_ANSI_SGR: {
        struct ansi_callback_1arg *ac_data = data;        
        printf("got a graphics routine number %d\n", ac_data->arg);
        break;
    }
    }
}

static void
rec_echo_command(int type)
{
    printf("server says it will echo: %d\n", type);
    printf("stop echoing for now\n");
}

struct telnetp *
telnet_connect(char *hostname, unsigned short port)
{
    struct telnetp *tn = telnetp_connect(hostname, port, telnet_callback);

    if(!tn) return NULL;

    telnetp_enable_option(tn, TO_ECHO, true, rec_echo_command);
    telnetp_enable_option(tn, TO_SUPRESS_GO_AHEAD, true, NULL);
    telnetp_enable_option(tn, TO_COMPRESS2, true, NULL);
    telnetp_enable_option(tn, TO_COMPRESS, true, NULL);

    return tn;
}

int
telnet_process(struct telnetp *tn)
{
    need_refresh = false;

    telnetp_process_incoming(tn);

    if(need_refresh)
        return true;
    
    return false;
}

void
telnet_close(struct telnetp *tn)
{
    telnetp_close(tn);
}

void
telnet_send(struct telnetp *tn, char *text)
{
    telnetp_send_line(tn, text, strlen(text));
}
