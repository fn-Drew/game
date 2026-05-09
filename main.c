#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

#define KEY_COUNT 4
#define SCREEN_HEIGHT 800
#define SCREEN_WIDTH 800

typedef enum {W, A, S, D} PlayerKey;

typedef enum {AXIS_X = 0, AXIS_Y = 1} Axis;
typedef struct {
  char code;
  int axis;
  int direction;
  int boundary;
} Key;

Key default_keys[KEY_COUNT] = {
  {'W', AXIS_Y, -1, 0},
  {'A', AXIS_X, -1, 0},
  {'S', AXIS_Y,  1, SCREEN_HEIGHT},
  {'D', AXIS_X,  1, SCREEN_WIDTH},
};
typedef struct {
  Key keys[KEY_COUNT];
  bool keys_pressed[KEY_COUNT];
  int radius;
  int speed;
  Vector2 position;
} Player;

void initializePlayer(Player *player) {
  player->position = (Vector2){(float)SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f};
  player->radius = 5;
  player->speed= 250;
  memcpy(player->keys, default_keys, sizeof(default_keys));
}

void updatePlayer(Player *player) {
  // creates a new pointer to player->position as a float
  // for accessing player position with player->keys.axis in the loop
  float *pos = (float *)&player->position;
  for(int key = 0; key < KEY_COUNT; key++){
    // keep our player's key presses updated
    player->keys_pressed[key] = IsKeyDown(player->keys[key].code);

    // temporary pointer for brevity
    Key *k = &player->keys[key];
    // accesses player->position through pos[0] or pos[1] (x or y)
    // adds the appropriate direction (-1 or 1, up/right or down/left) to the axis
    if(player->keys_pressed[key] && k->direction * ((pos[k->axis]) + k->direction * player->radius) < (k->boundary)){
      pos[k->axis] += k->direction * player->speed * GetFrameTime();
    }
  }
}

int main(void) {
  Player p1;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game window");
  SetTargetFPS(60);
  initializePlayer(&p1);

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:
    updatePlayer(&p1);

    BeginDrawing();

      ClearBackground(RAYWHITE);
      // DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), p1.position.x + 20, p1.position.y + 20, 20, BLACK);
      DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), 20, 20, 20, BLACK);
      DrawText(TextFormat("W:", p1.keys_pressed[W]), -20, 20, 20, BLACK);
      DrawCircleV(p1.position, 5, RED);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}