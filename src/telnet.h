#ifndef __TELNET_H__
#define __TELNET_H__

#include <gtk/gtk.h>

#include "telnetp.h"

struct telnetp *telnet_connect(char *hostname, unsigned short port);
void telnet_send(struct telnetp *tn, char *text);
int telnet_process(struct telnetp *tn);
void telnet_close(struct telnetp *tn);

void telnet_set_gtk_text_buffer(GtkTextBuffer *tb);
void telnet_set_gtk_vert_adj(GtkAdjustment *adj);

#endif  /* __TELNET_H__ */
