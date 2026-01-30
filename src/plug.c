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

// NOTE: problem is that height is stored in 8 bits, so height is really clamped
// so need to normalize and scale corre2tly

#define SLOPE_FLAT (Color){93, 133, 19, 255}
#define SLOPE_GENTLE (Color){65, 119, 0, 255}
#define SLOPE_STEEP (Color){78, 124, 21, 255}
#define SLOPE_VERY_STEEP (Color){129, 100, 31, 255}
#define SLOPE_EXTREME (Color){114, 90, 35, 255}
#define SEA_COLOR BLUE
#define SAND_COLOR (Color){219, 215, 110, 255}

#define MAP_WIDTH 512
#define MAP_HEIGHT 512

#define MAP_MESH_WIDTH 512
#define MAP_MESH_HEIGHT 512

#define SEA_OFFSET 10.0f
// FALLOFF (0.f-1.f)
#define FALLOFF_DENSITY 0.4f 
#define FALLOFF_HEIGHT 10.f

#define NOISE_HEIGHT 120.f

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
  bool show_textures;
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

  s->show_textures = true;

  fnl_state noise = fnlCreateState();
  noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
  noise.fractal_type = FNL_FRACTAL_FBM;
  noise.octaves = 2;
  noise.lacunarity = 3.05f;
  noise.weighted_strength = 2.06f;

  Image falloff = GenImageGradientRadial(MAP_WIDTH, MAP_HEIGHT, FALLOFF_DENSITY, WHITE, BLACK);
  Color *falloff_pixels = LoadImageColors(falloff);

  unsigned char *heightmap_pixels = malloc(MAP_WIDTH * MAP_HEIGHT * sizeof(unsigned char));

  size_t index = 0;
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      float raw_noise = fnlGetNoise2D(&noise, x, y);
      float normalized_noise = (raw_noise + 1.0f) / 2.0f;
      float falloff_value = falloff_pixels[index].r / 255.0f;
      float height = NOISE_HEIGHT * normalized_noise * falloff_value + falloff_value * FALLOFF_HEIGHT - SEA_OFFSET;
      heightmap_pixels[index++] = (unsigned char)fminf(fmaxf(height, 0.0f), 255.0f);
    }
  }

  UnloadImageColors(falloff_pixels);
  UnloadImage(falloff);

  Image heightmap = {
    .data = heightmap_pixels,
    .width = MAP_WIDTH,
    .height = MAP_HEIGHT,
    .format =  PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    .mipmaps = 1
  };
 
  // ImageBlurGaussia(&heightmap, 2);

  s->background = SKYBLUE;

  Image color = GenImageColor(MAP_WIDTH, MAP_HEIGHT, BLACK);

  Color *pixels = LoadImageColors(heightmap);

  float world_height_per_color = 20.0f / 255.0f;
  float world_dist_per_pixel = (float)MAP_MESH_WIDTH / MAP_WIDTH;
  float slope_scale_factor = world_height_per_color / world_dist_per_pixel;

  index = 0;
  for (int y = 0; y < heightmap.height; y++) {
    for (int x = 0; x < heightmap.width; x++) {
      index++;
      float raw_slope = calculate_slope_at_point(pixels, x, y, MAP_WIDTH, MAP_HEIGHT);

      if (pixels[index].r == 0) {
	ImageDrawPixel(&color, x, y, SEA_COLOR);
	continue;
      }

      if (pixels[index].r < 10) {
	ImageDrawPixel(&color, x, y, SAND_COLOR);
	continue;
      }
      
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

  if (IsKeyPressed(KEY_T)) {
    s->show_textures = !s->show_textures;
  }

  UpdateCamera(&s->camera, CAMERA_ORBITAL); 
  // UpdateCamera(&s->camera, CAMERA_FREE); 
}

void plug_draw(void *state) {
  State *s = (State*)state;

  BeginDrawing();
  ClearBackground(s->background);

  BeginMode3D(s->camera);

  DrawModel(s->landscape, (Vector3){-MAP_MESH_WIDTH/2, 0, -MAP_MESH_HEIGHT/2 }, 1.f, WHITE);
  // DrawGrid(20, 10.0f);

  EndMode3D();

  if (s->show_textures) {
    static float scale = 0.45f;
    DrawTextureEx(s->heightmap, Vector2Zero(), 0, scale, WHITE);
  }

  DrawFPS(10, 10);
  EndDrawing();
}

void plug_deinit(void *state) { 
  State *s = (State*)state;

  UnloadModel(s->landscape);
}
