#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define PLAYER_ACCELERATION 2500
#define SPARK_COUNT 100
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
  int code;
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
Key default_keys_p2[KEY_COUNT] = {
  {KEY_UP, AXIS_Y, -1, 0},
  {KEY_LEFT, AXIS_X, -1, 0},
  {KEY_DOWN, AXIS_Y,  1, SCREEN_HEIGHT},
  {KEY_RIGHT, AXIS_X,  1, SCREEN_WIDTH},
};

typedef struct {
  Key keys[KEY_COUNT];
  bool keys_pressed[KEY_COUNT];
  int radius;
  Vector2 position;
  Vector2 velocity;
  float fire_timer;
} Player;

typedef struct {
  Vector2 position;
  Vector2 direction;
  float speed;
  bool active;
} Projectile;

typedef struct {
  Vector2 position;
  float stay_time;
  bool active;
} Spark;

// arrays
Rectangle walls[WALL_COUNT] = {
  0
};

Projectile projectiles[PROJECTILE_COUNT] = {
  0
};

Spark sparks[SPARK_COUNT] = {
  0
};

// functions
void initializePlayer(Player *player) {
  player->position = (Vector2){(float)SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f};
  player->radius = 5;
  player->fire_timer = 0.0f;
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
  direction = Vector2Normalize(direction); // normalizes diagonal movement
  // gets velocity from direction and acceleration
  player->velocity = Vector2Add(player->velocity, Vector2Scale(
    direction, PLAYER_ACCELERATION * GetFrameTime()));
  player->velocity = Vector2Scale(player->velocity, 0.85f); // slows dowm movement with friction
  // moves player with velocity
  pos[0] += player->velocity.x * GetFrameTime();
  pos[1] += player->velocity.y * GetFrameTime();
}

void resolveCollision(Player *player, Rectangle rec){
  float left_overlap = (rec.x + rec.width) - (player->position.x - player->radius);
  float right_overlap = (player->position.x + player->radius) - (rec.x);
  float top_overlap = (rec.y + rec.height) - (player->position.y - player->radius);
  float bottom_overlap = (player->position.y + player->radius) - (rec.y);
  // checks which overlap is smallest and pushes player out of wall
  float min_overlap = fminf(fminf(left_overlap, right_overlap), fminf(top_overlap, bottom_overlap));
  if(min_overlap == left_overlap)
    player->position.x = rec.x + rec.width + player->radius;
else if(min_overlap == right_overlap)
    player->position.x = rec.x - player->radius;
else if(min_overlap == top_overlap)
    player->position.y = rec.y + rec.height + player->radius;
else if(min_overlap == bottom_overlap)
    player->position.y = rec.y - player->radius;
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

void spawnSpark(Spark *sparks, Vector2 position){
  for (int s = 0; s < SPARK_COUNT; s++)
  {
    if (sparks[s].active == false){
        sparks[s].position = position;
        sparks[s].stay_time = 0.1f;
        sparks[s].active = true;
        break;
    }
  }
}

int main(void) {
  Player p1;
  Player p2;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game window");
  // sound
  InitAudioDevice();
  Sound gunshot = LoadSound("gunshot.wav");
  Sound impact = LoadSound("impact.wav");

  SetTargetFPS(60);
  initializePlayer(&p1);
  initializePlayer(&p2);
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
    
    p1.fire_timer += GetFrameTime();

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
      if (p1.fire_timer >= 1.0f / 15.0f)
      {
        spawnProjectile(projectiles, &p1);
        PlaySound(gunshot);
        p1.fire_timer = 0.0f;
      }
    }
    //projectile update loop
    for (int i = 0; i < PROJECTILE_COUNT; i++)
    {
      if(projectiles[i].active == true){
      projectiles[i].position = Vector2Add(projectiles[i].position, 
        Vector2Scale(projectiles[i].direction, projectiles[i].speed * GetFrameTime()));
        // deactivates projectiles if they go off screen
        if (projectiles[i].position.x > 800 || projectiles[i].position.x < 0 
        || projectiles[i].position.y > 800 || projectiles[i].position.y < 0)
        {
        projectiles[i].active = false;
        }
      }
    }
    // projectile wall collision check loop
    for (int wall = 0; wall < char_count; wall++)
    {
      for (int p = 0; p < PROJECTILE_COUNT; p++)
      {
        if(projectiles[p].active == true){
          if(CheckCollisionPointRec(projectiles[p].position, walls[wall]) == true){
            spawnSpark(sparks, projectiles[p].position);
            PlaySound(impact);
            projectiles[p].active = false;
          }
        }
      }     
    }
    // spark effect update loop
    for (int s = 0; s < SPARK_COUNT; s++)
    {
      if(sparks[s].active == true){
        sparks[s].stay_time -= GetFrameTime();
        if(sparks[s].stay_time <= 0){
            sparks[s].active = false;
        }
      }
    }
    
    

    // moves player out of walls to previous position
    for(int wall = 0; wall < char_count; wall++){
    if(CheckCollisionCircleRec(p1.position, p1.radius, walls[wall])){ 
      resolveCollision(&p1, walls[wall]); 
      }
    }

    BeginDrawing();

      ClearBackground(LIGHTGRAY);
      // DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), p1.position.x + 20, p1.position.y + 20, 20, BLACK);
      DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), 20, 20, 20, BLACK);
      DrawText(TextFormat("W:", p1.keys_pressed[W]), -20, 20, 20, BLACK);
      DrawCircleV(p1.position, 5, RED);
      DrawCircleV(p2.position, 5, BLUE);
      
      for(int wall = 0; wall < char_count; wall++){
        DrawRectangleRec(walls[wall], DARKGRAY);
      };
      // draws projectiles with a tracer
      for (int i = 0; i < PROJECTILE_COUNT; i++){ 
        if(projectiles[i].active == true){ 
          DrawLineEx(projectiles[i].position, Vector2Subtract(
            projectiles[i].position, Vector2Scale(projectiles[i].direction, 20)),
           3.0f, ORANGE); 
          }
      }
      // draws sparks when projectiles hit a wall
      for (int s = 0; s < SPARK_COUNT; s++){
        if(sparks[s].active == true){
          DrawCircleV(sparks[s].position, 3, ORANGE);
        }
      }
    EndDrawing();
  }
  UnloadSound(gunshot);
  UnloadSound(impact);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}