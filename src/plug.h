#ifndef PLUG_H
#define PLUG_H

#include <stddef.h>
#include <raylib.h>

typedef struct State {
    int width;
    int height;
    Camera camera;
    Shader shader;
    Color background;
    Texture2D heightmap;
    Model landscape;
    bool show_textures;
} State;

typedef struct {
    void (*plug_init)(void *state);
    void (*plug_update)(void *state);
    void (*plug_draw)(void *state);
    void (*plug_deinit)(void *state);
} Plug;

#endif // PLUG_H
