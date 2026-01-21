#ifndef HOTRELOAD_H
#define HOTRELOAD_H

#include "plug.h"

int plug_poll_try_rebuild(Plug *plug);

int plug_load(Plug *plug);

int plug_reload(Plug *plug);

int plug_unload(Plug *plug);

#endif // HOTRELOAD_H
