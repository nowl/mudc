#include <stdio.h>
#include <stdbool.h>

#include "telnet.h"

static GtkTextBuffer *text_buffer = NULL;

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb)
{
    text_buffer = tb;
}


static void
telnet_callback(int type, void *data)
{
    switch(type) {
    case TC_LINE_FEED:
        gtk_text_buffer_insert_at_cursor(text_buffer, "\n", 1);
        break;
    case TC_CARRIAGE_RETURN:
        //gtk_text_buffer_insert_at_cursor(text_buffer, "\r", 1);
        break;
    case TC_ASCII: {
        struct ascii_callback *ac_data = data;
        char tmp[1] = {ac_data->c};
        gtk_text_buffer_insert_at_cursor(text_buffer, tmp, 1);
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

void
telnet_process(struct telnetp *tn)
{
    telnetp_process_incoming(tn);
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
