#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_MAIN_WINDOW_FONT   "MainWindowFont"
#define CONFIG_TEXT_ENTRY_FONT    "TextEntryFont"

void config_read();
char *config_get(char *key);
void config_set(char *key, char *value);

#endif  /* __CONFIG_H__ */
