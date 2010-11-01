#ifndef __TELNET_H__
#define __TELNET_H__

#include <gtk/gtk.h>

#include "telnetp.h"

struct telnetp *telnet_connect(char *hostname, unsigned short port);
void telnet_process(struct telnetp *tn);
void telnet_close(struct telnetp *tn);

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb);

#endif  /* __TELNET_H__ */
