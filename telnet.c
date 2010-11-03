#include <stdio.h>
#include <stdbool.h>

#include "telnet.h"

static GtkTextBuffer *text_buffer = NULL;

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb)
{
    text_buffer = tb;
}

static void
rec_line_feed()
{
    gtk_text_buffer_insert_at_cursor(text_buffer, "\n", 1);
}

static void
rec_car_ret()
{
    //gtk_text_buffer_insert_at_cursor(text_buffer, "\r", 1);
}

static void
rec_bell()
{
}

static void
rec_backspace()
{
}

static void
rec_hor_tab()
{
}

static void
rec_vert_tab()
{
}

static void
rec_form_feed()
{
}

static void
rec_erase_line()
{
}

static void
rec_erase_char()
{
}

static void
rec_ascii(char c)
{
    char tmp[1] = {c};
    gtk_text_buffer_insert_at_cursor(text_buffer, tmp, 1);
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
    struct telnetp_cbs cbs = {rec_ascii,
                              NULL,
                              rec_line_feed,
                              rec_car_ret,
                              rec_bell,
                              rec_backspace,
                              rec_hor_tab,
                              rec_vert_tab,
                              rec_form_feed,
                              rec_erase_line,
                              rec_erase_char,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL};

    struct telnetp *tn = telnetp_connect(hostname, port, cbs);

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
