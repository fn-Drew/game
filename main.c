#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

#define KEY_COUNT 4

const int screenWidth = 800;
const int screenHeight = 800;

int default_keys[4] = {'W','A','S','D'};
typedef enum {W, A, S, D} PlayerKey;

typedef struct {
  int keys[KEY_COUNT];
  bool keys_pressed[KEY_COUNT];
  int radius;
  int speed;
  Vector2 position;
} Player;

void initializePlayer(Player *player) {
  for(int key = 0; key < KEY_COUNT; key++){
    player->keys[key] = default_keys[key];
  }
  player->position = (Vector2){(float)screenWidth/2.0f, (float)screenHeight/2.0f};
  player->radius = 5;
  player->speed= 250;
}

void updatePlayer(Player *player) {
  for(int key = 0; key < KEY_COUNT; key++){
    player->keys_pressed[key] = IsKeyDown(player->keys[key]);
  }
}

int main(void) {
  Player p1;
  initializePlayer(&p1);

  InitWindow(screenWidth, screenHeight, "game window");
  SetTargetFPS(60);

  const int speed = 250;
  int deltaCircle_radius = 5;

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:
    updatePlayer(&p1);

    if (p1.keys_pressed[W] && p1.position.y > 0 + p1.radius){
      p1.position.y -= GetFrameTime()*speed;
    }

    if (p1.keys_pressed[A] && p1.position.x > 0 + p1.radius){
      p1.position.x -= GetFrameTime()*speed;
    }

    if (p1.keys_pressed[S] && p1.position.y < screenHeight - p1.radius){
      p1.position.y += GetFrameTime()*speed;
    }

    if (p1.keys_pressed[D] && p1.position.x < screenWidth - p1.radius){
      p1.position.x += GetFrameTime()*speed;
    }

    BeginDrawing();

      ClearBackground(RAYWHITE);
      DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x + 20, p1.position.y + 20), 200, 200, 20, BLACK);
      DrawCircleV(p1.position, 5, RED);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}