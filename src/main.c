#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>

#include "plug.h"
#include "hotreload.h"

#define SCREEN_WIDTH  (int)(1920/(1))
#define SCREEN_HEIGHT (int)(1080/(1))

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

    Camera camera = (Camera){ 0 };
    camera.position = (Vector3){ 240.0f, 160.0f, 80.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    memcpy((void*)state + sizeof(int) * 2, &camera, sizeof(Camera));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Plugin");
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
