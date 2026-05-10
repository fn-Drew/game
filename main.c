#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define PROJECTILE_COUNT 100
#define TILE_WIDTH 80
#define TILE_HEIGHT 80
#define ROW_COUNT 10
#define COLUMN_COUNT 10
#define WALL_COUNT 100
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

typedef struct {
  Vector2 position;
  Vector2 direction;
  float speed;
  bool active;
} Projectile;

// arrays
Rectangle walls[WALL_COUNT] = {
  0
};

Projectile projectiles[PROJECTILE_COUNT] = {
  0
};

// functions
void initializePlayer(Player *player) {
  player->position = (Vector2){(float)SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f};
  player->radius = 5;
  player->speed= 250;
  for(int key = 0; key < KEY_COUNT; key++){
    // maps default keys and their axis + direction onto the new players keys
    player->keys[key].code = default_keys[key].code;
    player->keys[key].axis = default_keys[key].axis;
    player->keys[key].direction = default_keys[key].direction;
    player->keys[key].boundary = default_keys[key].boundary;
  }
}

void updatePlayer(Player *player) {
  Vector2 direction = {0, 0};
  float *dir = (float *)&direction;
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
    if(player->keys_pressed[key] && k->direction * (pos[k->axis]) + player->radius < (k->boundary)){
      dir[k->axis] += k->direction;
    }
  }
  // normalizes diagonal movement
  direction = Vector2Normalize(direction);
  pos[0] += direction.x * player->speed * GetFrameTime();
  pos[1] += direction.y * player->speed * GetFrameTime();
}

void spawnProjectile(Projectile *projectiles, Player *player){
  for (int proj = 0; proj < PROJECTILE_COUNT; proj++)
  {
    if (projectiles[proj].active == false)
    {
      projectiles[proj].position = player->position;
      projectiles[proj].direction = (Vector2Normalize(
        Vector2Subtract(
          GetMousePosition(), player->position)
        )
      );
      projectiles[proj].speed = 2000;
      projectiles[proj].active = true;
      break;
    }
  }
}

int main(void) {
  Player p1;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game window");
  SetTargetFPS(60);
  initializePlayer(&p1);
  // loads the map
  FILE* map = fopen("map.txt", "r");
  if (map == NULL) {
    return 0;
  }
  // generates the map from the file
  int char_count = 0;
  for(int rows = 0; rows < ROW_COUNT; rows++){
    for(int columns = 0; columns < COLUMN_COUNT; columns++){
      // adds rectangle to walls array if character is a 1
      char map_char = fgetc(map);
        if(map_char == '1'){ 
          walls[char_count] = (Rectangle){
            columns * TILE_WIDTH, rows * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT}; 
          char_count++; 
      }
    }
    fgetc(map); // consume newline character
  }
  

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:
    Vector2 current_position = p1.position;

    updatePlayer(&p1);
    
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
    spawnProjectile(projectiles, &p1);
    }
    //projectile update loop
    for (int i = 0; i < PROJECTILE_COUNT; i++)
    {
      projectiles[i].position = Vector2Add(projectiles[i].position, 
        Vector2Scale(projectiles[i].direction, projectiles[i].speed * GetFrameTime()));
        // deactivates projectiles if they go off screen
        if (projectiles[i].position.x > 800 || projectiles[i].position.x < 0 
        || projectiles[i].position.y > 800 || projectiles[i].position.y < 0)
        {
        projectiles[i].active = false;
        }
    }
    

    // moves player out of walls to previous position
    for(int wall = 0; wall < char_count; wall++){
    if(CheckCollisionCircleRec(p1.position, p1.radius, walls[wall])){ 
      p1.position = current_position; 
      }
    }

    BeginDrawing();

      ClearBackground(RAYWHITE);
      // DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), p1.position.x + 20, p1.position.y + 20, 20, BLACK);
      DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), 20, 20, 20, BLACK);
      DrawText(TextFormat("W:", p1.keys_pressed[W]), -20, 20, 20, BLACK);
      DrawCircleV(p1.position, 5, RED);
      
      for(int wall = 0; wall < char_count; wall++){
        DrawRectangleRec(walls[wall], DARKGRAY);
      };
      // draws projectiles with a tracer
      for (int i = 0; i < PROJECTILE_COUNT; i++){ 
        if(projectiles[i].active == true){ 
          DrawLineV(projectiles[i].position, Vector2Subtract(
            projectiles[i].position, Vector2Scale(projectiles[i].direction, 20)),
           YELLOW); 
          }
      }
    EndDrawing();
  }
  CloseWindow();

  return 0;
}