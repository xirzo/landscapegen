#include "hotreload.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <sys/stat.h>

static const char *rebuild_exe_name = "./nob";
static const char *lib_name = "./build/plug.so";

static void* lib = NULL;

int plug_rebuild() {
    int code = system(rebuild_exe_name);

    if (code != 0) {
        fprintf(stderr, "ERROR: Failed to rebuild linux plug\n");
        return code;
    }

    return 0;
}

int plug_load(Plug *plug) {
    printf("Linux plug loading...\n");

    int rebuild_code;
    if ((rebuild_code = plug_rebuild()) != 0) {
        return rebuild_code;
    }

    lib = dlopen(lib_name, RTLD_NOW);

    if (lib == NULL) {
        fprintf(stderr, "ERROR: Failed to load library after rebuild: %s\n", dlerror());
        return 1;
    }

    // TODO: refactor with XMacro?
    plug->plug_init = dlsym(lib, "plug_init");

    char *error;
    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "ERROR: Failed to load %s function: %s\n", "plug_init", error);
        return 1;
    }

    plug->plug_draw = dlsym(lib, "plug_draw");

    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "ERROR: Failed to load %s function: %s\n", "plug_draw", error);
        return 1;
    }

    plug->plug_update = dlsym(lib, "plug_update");

    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "ERROR: Failed to load %s function: %s\n", "plug_update", error);
        return 1;
    }

    plug->plug_deinit = dlsym(lib, "plug_deinit");

    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "ERROR: Failed to load %s function: %s\n", "plug_deinit", error);
        return 1;
    }

    printf("Successfully loaded the plug\n");
    return 0;
}

int plug_reload(Plug *plug) {
    if (lib == NULL) {
        fprintf(stderr, "ERROR: lib is not loaded yet\n");
        return 1;
    }

    assert(plug_unload(plug) == 0);
    assert(plug_load(plug) == 0);

    printf("Successfully reloaded the plug\n");
    return 0;
}

int plug_unload(Plug *plug) {
    (void)plug;
    if (lib == NULL) {
        fprintf(stderr, "ERROR: lib is not loaded yet\n");
        return 1;
    }

    int dlclose_result = dlclose(lib);
    if (dlclose_result != 0) {
        fprintf(stderr, "ERROR: Failed to close plug: %s\n", dlerror());
        return dlclose_result;
    }

    plug->plug_init = NULL;
    plug->plug_update = NULL;
    plug->plug_draw = NULL;
    plug->plug_deinit = NULL;
    
    printf("Successfully unloaded the plug\n");
    return 0;
}
