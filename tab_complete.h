#ifndef __TAB_COMPLETE_H__
#define __TAB_COMPLETE_H__

/* Loads the word list from disk. */
void tab_complete_set_wordlist_file(char *filename);

/* Saves the word list to disk. */
void tab_complete_save();

/* Returns an array of strings of length r_len that match the partial
 * word. Returns NULL if no matches were found. */
char **tab_complete_find_matches(char *partial, int *r_len);

/* Adds the given word to the word list if it does not already exist
 * in the list. */
void tab_complete_add_word(char *word);

/* Adds the given words to the word list if they does not already
 * exist in the list. Groups the words based any alphanumeric
 * characters, underscores, and hyphens. */
void tab_complete_add_sentence(char *sentence);

#endif  /* __TAB_COMPLETE_H__ */
