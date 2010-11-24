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

    /* create base directories if they don't exist */
    char *home_dir = getenv("HOME");
    int len = strlen(home_dir) + MAX_LINE_LEN + 18 + 1;
    char *tab_complete_name = malloc(sizeof(*tab_complete_name) * len);
    
    snprintf(tab_complete_name, len, "%s/.mudc", home_dir);
    mkdir(tab_complete_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    snprintf(tab_complete_name, len, "%s/.mudc/worlds", home_dir);
    mkdir(tab_complete_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int
main(int argc, char *argv[])
{    
    init_mudc();

    config_read();

    gui_init(&argc, &argv);

    macros_init();

    gtk_main();

    telnet_close(MUDC.telnet);

    tab_complete_save();
    
    return 0;
}
