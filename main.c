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

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:

    BeginDrawing();

      ClearBackground(RAYWHITE);
      DrawText("Hello world!", 200, 200, 20, BLACK);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}