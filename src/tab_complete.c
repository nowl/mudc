#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mudc.h"

#define MAX_WORD_SIZE     512
#define INIT_DICT_SIZE      4

static char *source_filename = NULL;
static char **word_list = NULL;
static size_t word_list_c = 0;
static int word_list_i = 0;

static void
add_word(char *word)
{
    /* don't allow duplicates */

    int len;
    char **results = tab_complete_find_matches(word, &len);
    if(results)
    {
        int i;
        for(i=0; i<len; i++)
            if( strcasecmp(results[i], word) == 0 )
                return;
    }

    /* go ahead and add it */

    if(word_list_i + 1 == word_list_c)
        word_list = memory_grow_to_size(word_list, sizeof(*word_list), &word_list_c, word_list_c * 2);
    
    word_list[word_list_i++] = strdup(word);
    //printf("adding '%s'\n", word);
}

static void
read_source_file()
{
    FILE *fin = fopen(source_filename, "r");
    if( !fin)
        return;

    while( !feof(fin) )
    {
        char tmp[MAX_WORD_SIZE];
        if( fscanf(fin, "%s", tmp) != 1 )
            break;
        add_word(tmp);
    }

    fclose(fin);
}

void
free_words_in_dict()
{
    if(word_list)
    {
        int i;
        for(i=0; i<word_list_i; i++)
            free(word_list[i]);
        word_list_i = 0;
    }
}

static int
b_search(char *p, int min, int max, int i)
{
    //printf("looking at %d, %d, %d\n", min, max, i);
    //printf("comparing '%s' and '%s'\n", p, word_list[i]);
    
    int p_len = strlen(p);
    int w_len = strlen(word_list[i]);
    int n_min, n_max, n_i;
    
    if(p_len <= w_len)
    {
        int res = strncasecmp(p, word_list[i], p_len);
        if(res == 0) return i;
        else if(res > 0) {n_min = i; n_max = max; n_i = (max+i)/2;}
        else {n_min = min; n_max = i; n_i = (min+i)/2;};
    }
    else
    {
        int res = strncasecmp(p, word_list[i], w_len);
        if(res >= 0) {n_min = i; n_max = max; n_i = (max+i)/2;}
        else {n_min = min; n_max = i; n_i = (min+i)/2;};
    }

    if(n_max != max || n_min != min || n_i != i)
        return b_search(p, n_min, n_max, n_i);
    else
        return -1;
}

static int
find(char *p)
{
    if(word_list_i == 0)
        return -1;

    return b_search(p, 0, word_list_i, word_list_i/2);
}

static int
str_cmp(const void *a, const void *b)
{
    char *t1 = *(char **)a;
    char *t2 = *(char **)b;

    int t1_len = strlen(t1);
    int t2_len = strlen(t2);

    if(t1_len <= t2_len)
    {
        int res = strncasecmp(t1, t2, t1_len);
        if(res == 0) return -1;
        else return strncasecmp(t1, t2, t1_len);
    }
    else
    {
        int res = strncasecmp(t1, t2, t2_len);
        if(res == 0) return 1;
        else return strncasecmp(t1, t2, t2_len);
    }
}

/********************************** API ******************************/

void
tab_complete_set_wordlist_file(char *filename)
{
    printf("setting wordlist file to '%s'\n", filename);

    free_words_in_dict();

    if(source_filename) 
        free(source_filename);

    source_filename = strdup(filename);

    if(word_list_c == 0)
    {
        word_list = malloc(sizeof(*word_list) * INIT_DICT_SIZE);
        word_list_c = 4;
    }

    read_source_file();
}

char **
tab_complete_find_matches(char *partial, int *r_len)
{
    /* this will guarantee that we are in the right place */
    int ind = find(partial);
    if(ind == -1)
        return NULL;

    *r_len = 1;

    /* find the rest of the matches in this region */
    int p_len = strlen(partial);
    int min_match = ind;
    int i;
    
    /* backwards */
    for(i=min_match; i>= 0; i--)
    {
        if(strlen(word_list[i]) < p_len)
            break;
    
        if(strncasecmp(word_list[i], partial, p_len) != 0)
            break;
    }

    min_match = i+1;

    /* forwards */
    for(i=min_match+1; i<word_list_i; i++)
    {
        if(strlen(word_list[i]) < p_len)
            break;

        if(strncasecmp(word_list[i], partial, p_len) != 0)
            break;

        (*r_len)++;
    }

    return &word_list[min_match];
}

void
tab_complete_add_word(char *word)
{
    add_word(word);
    qsort(word_list, word_list_i, sizeof(*word_list), str_cmp);

    //int i;
    //for(i=0; i<word_list_i; i++)
    //    printf("slot %d: %s\n", i, word_list[i]);
}

void
tab_complete_save()
{
    if(source_filename)
    {
        FILE *fout = fopen(source_filename, "w");
        int i;
        
        for(i=0; i<word_list_i; i++)
            fprintf(fout, "%s\n", word_list[i]);
        
        fclose(fout);
    }
}

void
tab_complete_add_sentence(char *sentence)
{
    char *tmp = strdup(sentence);
    char *str = strtok(tmp, TAB_DIVIDING_TOKENS);
    
    do {
        if(str) tab_complete_add_word(str);
        str = strtok(NULL, TAB_DIVIDING_TOKENS);
    } while(str);

    free(tmp);
}
