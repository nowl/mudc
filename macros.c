#include <gdk/gdkkeysyms.h>

#include "mudc.h"

static gboolean
accel_invoked(GtkAccelGroup *accel_group,
              GObject *acceleratable,
              guint keyval,
              GdkModifierType modifier)
{
    printf("pressed: %d, %d\n", keyval, GDK_q);

    return TRUE;
}

void
macros_init()
{
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    
/*     gtk_accel_group_connect_by_path(accel_group, */
/*                                     "macro1", */
/*                                     (GClosure *)accel_invoked); */

    GClosure *closure = g_cclosure_new(accel_invoked, NULL, NULL);

    gtk_accel_group_connect(accel_group,
                            GDK_q,
                            GDK_CONTROL_MASK,
                            0,
                            closure);

    gtk_window_add_accel_group(GTK_WINDOW(MUDC.widgets.main_window),
                               accel_group);
}
