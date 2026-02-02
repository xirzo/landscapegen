#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <string.h>

#include "plug.h"
#include "hotreload.h"

#define SCREEN_WIDTH  (int)(1920/(1.5f))
#define SCREEN_HEIGHT (int)(1080/(1.5f))

int main(void) {
    Plug plug = {0};
    if (plug_load(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to initially load the plug\n");
        return 1;
    }

    State *s = (State*)malloc(sizeof(State));
    if (!s) {
        fprintf(stderr, "ERROR: Failed to init the plug\n");
        plug.plug_deinit(s);
        return 1;
    }

    s->width = SCREEN_WIDTH;
    s->height = SCREEN_HEIGHT;

    s->camera = (Camera){ 0 };
    s->camera.position = (Vector3){ 240.0f, 160.0f, 80.0f };
    s->camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    s->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    s->camera.fovy = 45.0f;
    s->camera.projection = CAMERA_PERSPECTIVE;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Plugin");
    SetTargetFPS(60);

    Shader shader = LoadShader("assets/vert.glsl", "assets/frag.glsl");
    s->shader = shader;

    plug.plug_init(s);

    while(!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        if (IsKeyPressed(KEY_SPACE)) {
	    plug.plug_deinit(s);

	    int reload_plug_result = plug_reload(&plug);

	    if (reload_plug_result != 0) {
		fprintf(stderr, "ERROR: Failed to reload the plug\n");
		free(s);
		CloseWindow();
		UnloadShader(shader);
		return 1;
	    }

	    plug.plug_init(s);
	}

        plug.plug_update(s);
        plug.plug_draw(s);
    }

    plug.plug_deinit(s);

    if (plug_unload(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to unload the plug\n");
	CloseWindow();
        plug.plug_deinit(s);
	UnloadShader(shader);
        free(s);
        return 1;
    }

    UnloadShader(shader);
    CloseWindow();
    free(s);
    return 0;
}

// NOTE: maybe abstract out all of the raylib functions?
