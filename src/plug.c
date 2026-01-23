#include "plug.h"

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>
#include <raylib.h>
#include <stdlib.h>

typedef struct Player {
    Color color;
    Vector2 pos;
    Vector2 dir;
    Vector2 vel;
    float accel;
    float current_accel;
    float speed;
    float max_speed;
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
    s->player.color = RED;
    s->player.pos = (Vector2){ s->screen_width/2, s->screen_height/2 };
    s->player.accel = 50.0f;
    s->player.current_accel = 0.0f;
    s->player.speed = 90.0f;
    s->player.max_speed = 80.0f;

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(s->screen_width, s->screen_height, "Plugin");
    SetTargetFPS(60);
}

void plug_update(void *state) { 
    State *s = (State*)state;

    s->player.dir = (Vector2){0.0f, 0.0f};

    if (IsKeyDown(KEY_A)) {
        s->player.dir.x -= 1.0f;
    }
    if (IsKeyDown(KEY_D)) {
        s->player.dir.x += 1.0f;
    }
    if (IsKeyDown(KEY_W)) {
        s->player.dir.y -= 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
        s->player.dir.y += 1.0f;
    }

    Vector2 desired_vel = {0.0f, 0.0f};
    if (s->player.dir.x != 0.0f || s->player.dir.y != 0.0f) {
        s->player.dir = Vector2Normalize(s->player.dir);
        desired_vel = Vector2Scale(s->player.dir, s->player.speed);
    }

    Vector2 dv = Vector2Subtract(desired_vel, s->player.vel);
    float dt = GetFrameTime();

    float vel_magnitude = Vector2Length(s->player.vel);
    float drag = 0.5f;
    float effective_accel = s->player.accel - (drag * vel_magnitude);

    if (effective_accel < 0.0f) {
        effective_accel = 0.0f;
    }

    s->player.current_accel = effective_accel;

    s->player.vel = Vector2Add(s->player.vel, Vector2Scale(dv, effective_accel * dt));

    vel_magnitude = Vector2Length(s->player.vel);
    if (vel_magnitude > s->player.max_speed) {
        s->player.vel = Vector2Scale(Vector2Normalize(s->player.vel), s->player.max_speed);
    }
    
    s->player.pos = Vector2Add(s->player.pos, Vector2Scale(s->player.vel, dt));
}

void plug_draw(void *state) {
    State *s = (State*)state;
    BeginDrawing();
    ClearBackground(s->background);

    DrawText(TextFormat("Player position: %.2f, %.2f", s->player.pos.x, s->player.pos.y), 0, 0, 20, BLACK);
    DrawText(TextFormat("Player velocity: %.2f, %.2f", s->player.vel.x, s->player.vel.y), 0, 25, 20, BLACK);
    DrawText(TextFormat("Player acceleration: %.2f (base: %.2f)", s->player.current_accel, s->player.accel), 0, 50, 20, BLACK);

    DrawRectangle(s->player.pos.x - 50, s->player.pos.y - 50, 100, 100, s->player.color);

    EndDrawing();
}

void plug_deinit(void *state) { 
    (void)state;

    CloseWindow();
}
