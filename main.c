#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 800;

  InitWindow(screenWidth, screenHeight, "game window");
  SetTargetFPS(60);

  const int speed = 100;
  Vector2 deltaCircle = {0, (float)screenHeight/50.0f}; 

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:

    bool w_pressed = IsKeyDown(0x57);
    bool a_pressed = IsKeyDown(0x41);
    bool s_pressed = IsKeyDown(0x53);
    bool d_pressed = IsKeyDown(0x44);

    if (w_pressed){
      deltaCircle.y -= GetFrameTime()*speed;
    }

    if (a_pressed){
      deltaCircle.x -= GetFrameTime()*speed;
    }

    if (s_pressed){
      deltaCircle.y += GetFrameTime()*speed;
    }

    if (d_pressed){
      deltaCircle.x += GetFrameTime()*speed;
    }

    BeginDrawing();

      ClearBackground(RAYWHITE);
      DrawText("Hello world!", 200, 200, 20, BLACK);
      DrawCircleV(deltaCircle, 5, RED);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}