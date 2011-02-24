#include <stdio.h>
#include "tab_complete.h"

int main()
{
    tab_complete_set_wordlist_file("/home/nowl/.mudc/worlds/aardmud.org/tab");
    tab_complete_add_sentence("This is a simple test.");

    int results_len;
    char **results = tab_complete_find_matches("f", &results_len);
    if(results)
    {
        int i;
        for(i=0; i<results_len; i++)
            printf("found '%s'\n", results[i]);
    }
    else
    {
        printf("no results found\n");
    }

    tab_complete_save();

    return 0;
}
