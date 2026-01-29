#include "plug.h"

#define FNL_IMPL
#include "FastNoiseLite.h"

#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include <rlgl.h>
#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <stdlib.h>

#define SLOPE_FLAT (Color){105, 140, 115, 255}
#define SLOPE_GENTLE (Color){93, 122, 101, 255}
#define SLOPE_STEEP (Color){91, 94, 92, 255}
#define SLOPE_VERY_STEEP (Color){70, 71, 70, 255}
#define SLOPE_EXTREME (Color){48, 48, 48, 255}

#define MAP_WIDTH 250
#define MAP_HEIGHT 250

#define MAP_MESH_WIDTH 300
#define MAP_MESH_HEIGHT 300

#define MAP_FIRST_PERLIN_SCALE 2.50f
#define MAP_SECOND_PERLIN_SCALE 10.0f

#define HEIGHT_MIN 0.0f
#define HEIGHT_MAX 300.0f

// TODO: add painted texture (like in PEAK for instance)

typedef struct State {
  // do not move width, height, camera, as they are set by memset in main  :)
  // I just neither want it to be non-opaque struct nor create functionа
  // to set values directly (may do that with xmacro)
  int width;
  int height;
  Camera camera;
  Color background;
  Texture2D heightmap;
  Model landscape;

  Texture2D h1;
  Texture2D h2;
} State;

size_t plug_state_size(void) {
  return sizeof(State);
}

float calculate_slope_at_point(Color *pixels, int x, int y, int width, int height) {
    int left = (x > 0) ? (y * width + (x - 1)) : (y * width + x);
    int right = (x < width - 1) ? (y * width + (x + 1)) : (y * width + x);
    int top = (y > 0) ? ((y - 1) * width + x) : (y * width + x);
    int bottom = (y < height - 1) ? ((y + 1) * width + x) : (y * width + x);
    
    float h_center = pixels[y * width + x].r;
    float h_left = pixels[left].r;
    float h_right = pixels[right].r;
    float h_top = pixels[top].r;
    float h_bottom = pixels[bottom].r;
    
    float dx = (h_right - h_left) / 2.0f;
    float dy = (h_bottom - h_top) / 2.0f;
    
    return sqrtf(dx * dx + dy * dy);
}

Color calculate_color_by_slope(float slope) {
    const float GENTLE_SLOPE = 0.1f;
    const float MODERATE_SLOPE = 0.25f;
    const float STEEP_SLOPE = 0.5f;
    const float VERY_STEEP_SLOPE = 0.75f;
    
    if (slope < GENTLE_SLOPE) {
        return SLOPE_FLAT;
    }
    else if (slope < MODERATE_SLOPE) {
        float t = (slope - GENTLE_SLOPE) / (MODERATE_SLOPE - GENTLE_SLOPE);
        return ColorLerp(SLOPE_FLAT, SLOPE_GENTLE, t);
    }
    else if (slope < STEEP_SLOPE) {
        float t = (slope - MODERATE_SLOPE) / (STEEP_SLOPE - MODERATE_SLOPE);
        return ColorLerp(SLOPE_GENTLE, SLOPE_STEEP, t);
    }
    else if (slope < VERY_STEEP_SLOPE) {
        float t = (slope - STEEP_SLOPE) / (VERY_STEEP_SLOPE - STEEP_SLOPE);
        return ColorLerp(SLOPE_STEEP, SLOPE_VERY_STEEP, t);
    }
    else {
        float t = (slope - VERY_STEEP_SLOPE) / (1.0f - VERY_STEEP_SLOPE);
        t = fminf(t, 1.0f);
        return ColorLerp(SLOPE_VERY_STEEP, SLOPE_EXTREME, t);
    }
}

void plug_init(void *state) { 
  State *s = (State*)state;

  fnl_state noise = fnlCreateState();
  noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
  noise.fractal_type = FNL_FRACTAL_FBM;
  noise.octaves = 5;
  noise.lacunarity = 3.05f;
  noise.weighted_strength = 2.06f;

  Color *heightmap_pixels = malloc(MAP_WIDTH * MAP_HEIGHT * sizeof(Color));

  size_t index = 0;
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      float height = fnlGetNoise2D(&noise, x, y);
      const int HEIGHT_INCREASE_CONSTANT = 350;
      heightmap_pixels[index++] = (Color){ height * HEIGHT_INCREASE_CONSTANT, 0, 0, 255 };
    }
  }

  Image heightmap = {
    .data = heightmap_pixels,
    .width = MAP_WIDTH,
    .height = MAP_HEIGHT,
    .format =  PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    .mipmaps = 1
  };
 
  ImageBlurGaussian(&heightmap, 2);

  s->background = SKYBLUE;

  Image color = GenImageColor(MAP_WIDTH, MAP_HEIGHT, BLACK);

  Color *pixels = LoadImageColors(heightmap);

  float world_height_per_color = 20.0f / 255.0f;
  float world_dist_per_pixel = (float)MAP_MESH_WIDTH / MAP_WIDTH;
  float slope_scale_factor = world_height_per_color / world_dist_per_pixel;

  for (int y = 0; y < heightmap.height; y++) {
    for (int x = 0; x < heightmap.width; x++) {
      float raw_slope = calculate_slope_at_point(pixels, x, y, MAP_WIDTH, MAP_HEIGHT);
      
      float slope = raw_slope * slope_scale_factor;
      
      Color slope_color = calculate_color_by_slope(slope);
      
      ImageDrawPixel(&color, x, y, slope_color);
    }
  }

  UnloadImageColors(pixels);

  Mesh mesh = GenMeshHeightmap(heightmap, (Vector3){MAP_MESH_WIDTH, 20, MAP_MESH_HEIGHT});
  s->landscape = LoadModelFromMesh(mesh);

  Texture2D texture = LoadTextureFromImage(heightmap);
  UnloadImage(heightmap);

  Texture2D color_texture = LoadTextureFromImage(color);
  SetTextureFilter(color_texture, TEXTURE_FILTER_BILINEAR);
  UnloadImage(color);
  s->heightmap = texture;
  s->landscape.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = color_texture;
}

void plug_update(void *state) {
  State *s = (State*)state;
  UpdateCamera(&s->camera, CAMERA_ORBITAL); 
  // UpdateCamera(&s->camera, CAMERA_FREE); 
}

void plug_draw(void *state) {
  State *s = (State*)state;
  BeginDrawing();
  ClearBackground(s->background);

  BeginMode3D(s->camera);

  DrawModel(s->landscape, (Vector3){-MAP_MESH_WIDTH/2, 0, -MAP_MESH_HEIGHT/2 }, 1.f, WHITE);
  DrawGrid(20, 10.0f);

  EndMode3D();
  // void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
  float scale = 0.45f;
  DrawTextureEx(s->heightmap, Vector2Zero(), 0, scale, WHITE);

  DrawFPS(10, 10);
  EndDrawing();
}

void plug_deinit(void *state) { 
  State *s = (State*)state;

  UnloadModel(s->landscape);
  UnloadTexture(s->h1);
  UnloadTexture(s->h2);
}
