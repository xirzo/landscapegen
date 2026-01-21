#include <stdio.h>
#include <raylib.h>

#include "plug.h"
#include "hotreload.h"

int main(void) {
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(900, 600, "Plugin");
    SetTargetFPS(60);

    Plug plug = {0};

    if (plug_load(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to initially load the plug\n");
        CloseWindow();
        return 1;
    }

    while(!WindowShouldClose()) {
        plug_poll_try_rebuild(&plug);
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        if (IsKeyPressed(KEY_SPACE) && plug_reload(&plug) != 0) {
            fprintf(stderr, "ERROR: Failed to reload the plug\n");
            CloseWindow();
            return 1;
        }

        plug.plug_draw();
    }

    if (plug_unload(&plug) != 0) {
        fprintf(stderr, "ERROR: Failed to unload the plug\n");
        return 1;
    }

    CloseWindow();
    return 0;
}
