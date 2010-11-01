#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"

#define NUM_CONFIGS   32
#define MAX_LINE_LEN 512

struct config
{
    char *key, *value;
};

static struct config config_values[NUM_CONFIGS];
static int config_values_i = 0;

static char *fin_name = NULL;

void
read_values()
{
    FILE *fin = fopen(fin_name, "r");
    
    if(!fin)
    {
        /* file doesn't exist */
        return;
    }

    char key[MAX_LINE_LEN];
    char val[MAX_LINE_LEN];
    char *tmp_key = fgets(key, MAX_LINE_LEN, fin);
    char *tmp_val = fgets(val, MAX_LINE_LEN, fin);
    while(tmp_val)
    {
        config_values[config_values_i].key = strdup(key);
        config_values[config_values_i].value = strdup(val);

        /* erase the newline character */
        tmp_key = config_values[config_values_i].key;
        tmp_key[strlen(tmp_key)-1] = '\0';
        tmp_val = config_values[config_values_i].value;
        tmp_val[strlen(tmp_val)-1] = '\0';

        config_values_i++;

        tmp_key = fgets(key, MAX_LINE_LEN, fin);
        tmp_val = fgets(val, MAX_LINE_LEN, fin);
    }

    fclose(fin);
}

void
write_values()
{
    FILE *fin = fopen(fin_name, "w");
    
    int i;
    for(i=0; i<config_values_i; i++)
        fprintf(fin, "%s\n%s\n", config_values[i].key, config_values[i].value);

    fclose(fin);
}

void
config_read()
{
    char *home_dir = getenv("HOME");
    int len = strlen(home_dir) + 6 + 1;
    fin_name = malloc(sizeof(*fin_name) * len);
    snprintf(fin_name, len, "%s/.mudc", home_dir);
    read_values();
}

char *
config_get(char *key)
{
    int i;
    for(i=0; i<config_values_i; i++)
    {
        if( strcmp(key, config_values[i].key) == 0) {
            return config_values[i].value;
        }
    }

    return NULL;
}

void
config_set(char *key, char *value)
{
    int found = false;

    int i;
    for(i=0; i<config_values_i; i++) {
        if( strcmp(key, config_values[i].key) == 0) {
            found = true;
            break;
        }
    }

    if(found) {
        free(config_values[i].value);
        config_values[i].value = strdup(value);
    } else {
        config_values[config_values_i].key = strdup(key);
        config_values[config_values_i].value = strdup(value);

        config_values_i++;
    }

    /* write values out */
    write_values();
}
