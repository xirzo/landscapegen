#include "plug.h"

#include <raylib.h>
#include <stdlib.h>

typedef struct Player {
    Color color;
    int x;
    int y;
} Player;

typedef struct State {
    Color background;
    Player player;

    int screen_width;
    int screen_height;
} State;

size_t plug_state_size(void) {
    return sizeof(State);
}

void plug_init(void *state) { 
    State *s = (State*)state;
    s->background = WHITE;
    s->screen_width = 900;
    s->screen_height = 600;
    s->player.x = s->screen_width/2;
    s->player.y = s->screen_height/2;
    s->player.color = RED;

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(s->screen_width, s->screen_height, "Plugin");
    SetTargetFPS(60);
}

void plug_update(void *state) { 
    if (IsKeyDown(KEY_A)) {
        ((State*)state)->player.x -= 1;
    }
    if (IsKeyDown(KEY_D)) {
        ((State*)state)->player.x += 1;
    }
    if (IsKeyDown(KEY_W)) {
        ((State*)state)->player.y -= 1;
    }
    if (IsKeyDown(KEY_S)) {
        ((State*)state)->player.y += 1;
    }
}

void plug_draw(void *state) {
    State *s = (State*)state;
    BeginDrawing();
    ClearBackground(s->background);

    DrawRectangle(s->player.x, s->player.y, 100, 100, s->player.color);

    EndDrawing();
}

void plug_deinit(void *state) { 
    (void)state;

    CloseWindow();
}
