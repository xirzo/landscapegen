#include "plug.h"

#define RAYMATH_IMPLEMENTATION

#include <math.h>
#include <raymath.h>
#include <raylib.h>
#include <stdlib.h>

// Mesh GenMeshHeightmap(Image heightmap, Vector3 size);     

typedef struct State {
  // do not move width and height, as they are set by memset in main  :)
  // I just neither want it to be non-opaque struct nor create functions
  // to set values directly (may do that with xmacro)
  int width;
  int height;
  Color background;
} State;

size_t plug_state_size(void) {
  return sizeof(State);
}

void plug_init(void *state) { 
  State *s = (State*)state;
  s->background = BLACK;
}

void plug_update(void *state) {
  State *s = (State*)state;
}

void plug_draw(void *state) {
  State *s = (State*)state;
  BeginDrawing();
  ClearBackground(s->background);

  EndDrawing();
}

void plug_deinit(void *state) { 
  (void)state;
}
