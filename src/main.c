#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

#include "plug.h"
#include "hotreload.h"

int main(void) {
    Plug plug = {0};
    if (plug_load(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to initially load the plug\n");
        return 1;
    }

    void *state = malloc(plug.plug_state_size());
    if (!state) {
        fprintf(stderr, "ERROR: Failed to init the plug\n");
        plug.plug_deinit(state);
        return 1;
    }

    plug.plug_init(state);

    while(!WindowShouldClose()) {
        plug_poll_try_rebuild(&plug);

        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        if (IsKeyPressed(KEY_SPACE) && plug_reload(&plug) != 0) {
            fprintf(stderr, "ERROR: Failed to reload the plug\n");
            plug.plug_deinit(state);
            free(state);
            return 1;
        }

        plug.plug_update(state);
        plug.plug_draw(state);
    }

    plug.plug_deinit(state);

    if (plug_unload(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to unload the plug\n");
        plug.plug_deinit(state);
        free(state);
        return 1;
    }

    free(state);
    return 0;
}

// NOTE: maybe abstract out all of the raylib functions?
