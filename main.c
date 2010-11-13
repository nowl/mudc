#include <gtk/gtk.h>

#include <string.h>
#include <assert.h>

#include "mudc.h"

void init_mudc()
{
    MUDC.telnet = NULL;

    MUDC.buffers.command_history_c = 16;
    MUDC.buffers.command_history_i = 0;
    MUDC.buffers.command_history_p = 0;
    MUDC.buffers.command_history = malloc(sizeof(*MUDC.buffers.command_history) * MUDC.buffers.command_history_c);
        
    MUDC.buffers.text_to_find_c = 12;
    MUDC.buffers.text_to_find = malloc(sizeof(*MUDC.buffers.text_to_find) * MUDC.buffers.text_to_find_c);
}

int
main(int argc, char *argv[])
{    
    init_mudc();

    config_read();

    gui_init(&argc, &argv);

    gtk_main();

    telnet_close(MUDC.telnet);

    tab_complete_save();
    
    return 0;
}
