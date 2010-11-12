#include <gdk/gdkkeysyms.h>

#include "mudc.h"

gboolean
entry_handler_keypress(GtkWidget   *widget,
                       GdkEventKey *event,
                       gpointer     user_data)
{
    GtkTextBuffer *entry = MUDC.widgets.entry_buffer;

    if(event->keyval == GDK_KEY_Return)
    {
        /* Pressing return should clear the entry buffer and send
         * whatever is displayed to the mud server. */

        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry, &start);
        gtk_text_buffer_get_end_iter(entry, &end);

        gchar *text = gtk_text_buffer_get_text(entry, &start, &end, FALSE);

        /* clear the text */
        gtk_text_buffer_delete(entry, &start, &end);

        /* send the text */
        telnet_send(MUDC.telnet, text);

        /* save in history */
        if(MUDC.buffers.command_history_i == MUDC.buffers.command_history_c)
            MUDC.buffers.command_history = memory_grow_to_size(MUDC.buffers.command_history, 
                                                               sizeof(*MUDC.buffers.command_history),
                                                               &MUDC.buffers.command_history_c,
                                                               MUDC.buffers.command_history_c * 2);
        
        MUDC.buffers.command_history[MUDC.buffers.command_history_i++] = strdup(text);
        MUDC.buffers.command_history_p = MUDC.buffers.command_history_i;

        tab_complete_add_sentence(text);
    
        g_free(text);

        /* add a newline */
        /*
        text = "\n";
        gtk_text_buffer_insert_at_cursor(MUDC.widgets.text_buffer, text, strlen(text));
        */

        return TRUE;
    }
    else if(event->keyval == GDK_KEY_Up)
    {
        /* Move back to the previous entry in the history and display
         * it on the entry buffer. */

        MUDC.buffers.command_history_p--;
        if(MUDC.buffers.command_history_p < 0)
            MUDC.buffers.command_history_p = 0;

        char *cmd = MUDC.buffers.command_history[MUDC.buffers.command_history_p];
        
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry, &start);
        gtk_text_buffer_get_end_iter(entry, &end);
        
        gtk_text_buffer_delete(entry, &start, &end);

        gtk_text_buffer_insert_at_cursor(entry, cmd, strlen(cmd));

        return TRUE;
    }
    else if(event->keyval == GDK_KEY_Down)
    {
        /* Move forward to the next entry in the history and display
         * it on the entry buffer. */

        MUDC.buffers.command_history_p++;
        char *cmd;
        if(MUDC.buffers.command_history_p <= MUDC.buffers.command_history_i-1)
        {
            cmd = MUDC.buffers.command_history[MUDC.buffers.command_history_p];
        }
        else
        {
            MUDC.buffers.command_history_p = MUDC.buffers.command_history_i;

            cmd = "";
        }
        
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry, &start);
        gtk_text_buffer_get_end_iter(entry, &end);
        
        gtk_text_buffer_delete(entry, &start, &end);

        gtk_text_buffer_insert_at_cursor(entry, cmd, strlen(cmd));

        return TRUE;
    }
    else if(event->keyval == GDK_KEY_Tab)
    {
        /* This will try to autocomplete the text that is currently
         * under the cursor. If the word is in the middle of a
         * sentence the rest of the sentence will be deleted. */

        /* grab complete text */
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(entry, &start);
        gtk_text_buffer_get_end_iter(entry, &end);

        gchar *text = gtk_text_buffer_get_text(entry, &start, &end, FALSE);

        /* Find where the cursor is and find the previous "dividing"
         * character. This will then be in the range "i" to
         * "curs_pos" */
        gint curs_pos;
        g_object_get(entry, 
                     "cursor-position", &curs_pos,
                     NULL);

        char *tab_dividing_tokens = TAB_DIVIDING_TOKENS;
        int i;
        gboolean found_divider = FALSE;
        for(i=curs_pos-1; i>=0; i--)
        {
            int j;            
            for(j=0; j<sizeof(TAB_DIVIDING_TOKENS); j++)
                if(text[i] == tab_dividing_tokens[j])
                {
                    found_divider = TRUE;
                    break;
                }
            if(found_divider)
                break;
        }

        i++;        

        /* copy text to find into "text_to_find" */

        int size = curs_pos - i;
        MUDC.buffers.text_to_find = memory_grow_to_size(MUDC.buffers.text_to_find, 
                                                        sizeof(*MUDC.buffers.text_to_find),
                                                        &MUDC.buffers.text_to_find_c, 
                                                        size);

        memcpy(MUDC.buffers.text_to_find, &text[i], size);
        MUDC.buffers.text_to_find[curs_pos-i] = '\0';
        
        int num_results;
        char **results = tab_complete_find_matches(MUDC.buffers.text_to_find, &num_results);

        if(!results)
        {
            g_free(text);
            return TRUE;
        }

        if(num_results == 1)
        {
            /* if this is the only result then just replace the text with this */
            gtk_text_buffer_delete(entry, &start, &end);

            int len = strlen(results[0]) + i;
            MUDC.buffers.text_to_find = memory_grow_to_size(MUDC.buffers.text_to_find,
                                                            sizeof(*MUDC.buffers.text_to_find),
                                                            &MUDC.buffers.text_to_find_c, 
                                                            len + 1);
            memcpy(MUDC.buffers.text_to_find, text, i);
            memcpy(&MUDC.buffers.text_to_find[i], results[0], strlen(results[0]));
            MUDC.buffers.text_to_find[len] = '\0';
            gtk_text_buffer_insert_at_cursor(entry, MUDC.buffers.text_to_find, len);
        }
        else if(num_results > 1)
        {
            /* if there are multiple results then display each one on the text_buffer */
            int i;
            for(i=0; i<num_results; i++)
            {
                gtk_text_buffer_insert_at_cursor(MUDC.widgets.text_buffer, results[i], strlen(results[i]));
                char tmp[] = "\n";
                gtk_text_buffer_insert_at_cursor(MUDC.widgets.text_buffer, tmp, strlen(tmp));
                gtk_text_buffer_get_end_iter(MUDC.widgets.text_buffer, &end);
                gtk_text_buffer_move_mark(MUDC.widgets.text_buffer, MUDC.widgets.text_mark, &end);
                gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(MUDC.widgets.text_view), MUDC.widgets.text_mark);
            }
        }

        g_free(text);

        return TRUE;
    }

    return FALSE;    
}
