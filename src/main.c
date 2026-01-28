#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>

#include "plug.h"
#include "hotreload.h"

#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 900

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

    memset((void*)state, SCREEN_WIDTH, sizeof(int));
    memset((void*)state + sizeof(int), SCREEN_HEIGHT, sizeof(int));

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(900, 900, "Plugin");
    SetTargetFPS(60);

    plug.plug_init(state);

    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        if (IsKeyPressed(KEY_SPACE)) {
	    int reload_plug_result = plug_reload(&plug);

	    plug.plug_deinit(state);

	    if (reload_plug_result != 0) {
		fprintf(stderr, "ERROR: Failed to reload the plug\n");
		free(state);
		CloseWindow();
		return 1;
	    }

	    plug.plug_init(state);
	}

        plug.plug_update(state);
        plug.plug_draw(state);
    }

    plug.plug_deinit(state);

    if (plug_unload(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to unload the plug\n");
	CloseWindow();
        plug.plug_deinit(state);
        free(state);
        return 1;
    }

    CloseWindow();
    free(state);
    return 0;
}

// NOTE: maybe abstract out all of the raylib functions?
