#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "mudc.h"

static GtkTextBuffer *text_buffer = NULL;
static GtkAdjustment *vert_adj = NULL;

static gboolean need_refresh = FALSE;

static char *line_buffer = NULL;
static size_t line_buffer_c = 0;
static int line_buffer_i = 0;

static gboolean buffering_chars = FALSE;
static gboolean sgr_in_color = FALSE;
static char *sgr_tag_name;

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb)
{
    text_buffer = tb;

    /* initialize color tags */
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(text_buffer);
    GtkTextTag *tag = gtk_text_tag_new("text_color_1");
    g_object_set(tag, "foreground", ASGR_COLOR1_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_2");
    g_object_set(tag, "foreground", ASGR_COLOR2_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_3");
    g_object_set(tag, "foreground", ASGR_COLOR3_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_4");
    g_object_set(tag, "foreground", ASGR_COLOR4_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_5");
    g_object_set(tag, "foreground", ASGR_COLOR5_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_6");
    g_object_set(tag, "foreground", ASGR_COLOR6_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_7");
    g_object_set(tag, "foreground", ASGR_COLOR7_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("text_color_8");
    g_object_set(tag, "foreground", ASGR_COLOR8_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);

    tag = gtk_text_tag_new("bg_color_1");
    g_object_set(tag, "background", ASGR_COLOR1_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_2");
    g_object_set(tag, "background", ASGR_COLOR2_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_3");
    g_object_set(tag, "background", ASGR_COLOR3_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_4");
    g_object_set(tag, "background", ASGR_COLOR4_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_5");
    g_object_set(tag, "background", ASGR_COLOR5_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_6");
    g_object_set(tag, "background", ASGR_COLOR6_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_7");
    g_object_set(tag, "background", ASGR_COLOR7_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
    tag = gtk_text_tag_new("bg_color_8");
    g_object_set(tag, "background", ASGR_COLOR8_NORMAL, NULL);
    gtk_text_tag_table_add(tag_table, tag);
}

void telnet_set_gtk_vert_adj(GtkAdjustment *adj)
{
    vert_adj = adj;
}

static void dump_buffer()
{
    sgr_in_color = FALSE;
    buffering_chars = FALSE;
    
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
                
    gtk_text_buffer_insert_with_tags_by_name(text_buffer,
                                             &end,
                                             line_buffer,
                                             line_buffer_i,
                                             sgr_tag_name,
                                             NULL);
}

static void
telnet_callback(int type, void *data)
{
    switch(type) {
    case TC_LINE_FEED:

        if(!buffering_chars)
            gtk_text_buffer_insert_at_cursor(text_buffer, "\n", 1);
        else
        {
            if(line_buffer_c == line_buffer_i)
                line_buffer = memory_grow_to_size(line_buffer, sizeof(*line_buffer),  &line_buffer_c, line_buffer_i+1);
            line_buffer[line_buffer_i++] = '\n';
        }
        
        need_refresh = true;
        break;
    case TC_CARRIAGE_RETURN:
        //gtk_text_buffer_insert_at_cursor(text_buffer, "\r", 1);
        break;
    case TC_ASCII: {
        struct ascii_callback *ac_data = data;

        if(!buffering_chars)
        {
            char tmp[1] = {ac_data->c};
            gtk_text_buffer_insert_at_cursor(text_buffer, tmp, 1);
        }
        else
        {
            if(line_buffer_c == line_buffer_i)
                line_buffer = memory_grow_to_size(line_buffer, sizeof(*line_buffer), &line_buffer_c, line_buffer_i+1);
            line_buffer[line_buffer_i++] = ac_data->c;
        }
        break;
    }
    case TC_ANSI_SGR: {
        struct ansi_callback_1arg *ac_data = data;
        switch(ac_data->arg)
        {
        case ASGR_RESET:
            if(sgr_in_color) 
                dump_buffer();
            break;
        case ASGR_TEXT_COLOR_1:
        case ASGR_TEXT_COLOR_2:
        case ASGR_TEXT_COLOR_3:
        case ASGR_TEXT_COLOR_4:
        case ASGR_TEXT_COLOR_5:
        case ASGR_TEXT_COLOR_6:
        case ASGR_TEXT_COLOR_7:
        case ASGR_TEXT_COLOR_8:
        case ASGR_BG_COLOR_1:
        case ASGR_BG_COLOR_2:
        case ASGR_BG_COLOR_3:
        case ASGR_BG_COLOR_4:
        case ASGR_BG_COLOR_5:
        case ASGR_BG_COLOR_6:
        case ASGR_BG_COLOR_7:
        case ASGR_BG_COLOR_8:
        {
            if(sgr_in_color)
                dump_buffer();

            sgr_in_color = TRUE;
            
            switch(ac_data->arg) {
            case ASGR_TEXT_COLOR_1: sgr_tag_name = "text_color_1"; break;
            case ASGR_TEXT_COLOR_2: sgr_tag_name = "text_color_2"; break;
            case ASGR_TEXT_COLOR_3: sgr_tag_name = "text_color_3"; break;
            case ASGR_TEXT_COLOR_4: sgr_tag_name = "text_color_4"; break;
            case ASGR_TEXT_COLOR_5: sgr_tag_name = "text_color_5"; break;
            case ASGR_TEXT_COLOR_6: sgr_tag_name = "text_color_6"; break;
            case ASGR_TEXT_COLOR_7: sgr_tag_name = "text_color_7"; break;
            case ASGR_TEXT_COLOR_8: sgr_tag_name = "text_color_8"; break;
            case ASGR_BG_COLOR_1: sgr_tag_name = "bg_color_1"; break;
            case ASGR_BG_COLOR_2: sgr_tag_name = "bg_color_2"; break;
            case ASGR_BG_COLOR_3: sgr_tag_name = "bg_color_3"; break;
            case ASGR_BG_COLOR_4: sgr_tag_name = "bg_color_4"; break;
            case ASGR_BG_COLOR_5: sgr_tag_name = "bg_color_5"; break;
            case ASGR_BG_COLOR_6: sgr_tag_name = "bg_color_6"; break;
            case ASGR_BG_COLOR_7: sgr_tag_name = "bg_color_7"; break;
            case ASGR_BG_COLOR_8: sgr_tag_name = "bg_color_8"; break;
            };

            buffering_chars = TRUE;
            line_buffer_i = 0;
            
            break;            
        }
        };
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
    if(tn)
        telnetp_close(tn);
}

void
telnet_send(struct telnetp *tn, char *text)
{
    telnetp_send_line(tn, text, strlen(text));
}
