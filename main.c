#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define EXPLOSION_COUNT 20
#define GRENADE_COUNT 100
#define PICKUP_COUNT 20
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

typedef enum {
  P90,
  AK47,
  DESERT_EAGLE,
  BARRETT_50CAL,
  M4A1,
  KNIFE,
  SWORD,
  HAMMER,
  GRENADE,
  THROWING_KNIFE,
  SMOKE_GRENADE,
  LAND_MINE
} WeaponType;

typedef struct{
  WeaponType type;
  int damage;
  float cooldown;
  float accuracy;
  int magazine_size;
  int current_ammo;
  float reload_time;
  float explosion_radius;
  float fuse_time;
  float throw_speed;
  float range;
  float swing_arc;
  float attack_duration;
} Weapon;

typedef struct{
  Vector2 position;
  Vector2 direction;
  float speed;
  float fuse_timer;
  bool active;
  int damage;
  float explosion_radius;
  float friction;
  Vector2 prev_position;
  WeaponType type;
} Grenade;

typedef struct{
Vector2 position;
float radius;
float max_radius;
float timer;
float duration;
bool active;
} Explosion;

typedef enum {
  PICKUP_WEAPON,
  PICKUP_HEALTH,
  PICKUP_AMMO,
  PICKUP_ARMOR
} PickupType;

typedef struct {
  PickupType type;
  Vector2 position;
  bool active;
  WeaponType weapon;
  int value;
} Pickup;

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
  int health;
  Weapon weapons[2];
  int active_weapon;
  float reload_timer;
  bool is_reloading;
  int armor;
  bool is_attacking;
  float attack_timer;
  float attack_angle;
} Player;

typedef struct {
  Vector2 position;
  Vector2 prev_position;
  Vector2 direction;
  float speed;
  bool active;
  int damage;
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

Pickup pickups[PICKUP_COUNT] = {
  0
};

Grenade grenades[GRENADE_COUNT] = {
  0
};

Explosion explosions[EXPLOSION_COUNT] = {
  0
};

Weapon weapon_list[] = {
  {.type = P90, .damage = 15, .cooldown = 0.067f, .accuracy = 0.75f, .magazine_size = 50, .current_ammo = 50, .reload_time = 2.0f},
  {.type = AK47, .damage = 34, .cooldown = 0.1f, .accuracy = 0.85f, .magazine_size = 30, .current_ammo = 30, .reload_time = 1.5f},
  {.type = M4A1, .damage = 32, .cooldown = 0.085f, .accuracy = 0.95f, .magazine_size = 30, .current_ammo = 30, .reload_time = 1.5f},
  {.type = DESERT_EAGLE, .damage = 51, .cooldown = 0.21f, .accuracy = 0.95f, .magazine_size = 7, .current_ammo = 7, .reload_time = 1.5f},
  {.type = BARRETT_50CAL, .damage = 150, .cooldown = 1.0f, .accuracy = 1.0f, .magazine_size = 10, .current_ammo = 10, .reload_time = 2.5f},
  {.type = KNIFE, .damage = 34, .cooldown = 0.1f, .range = 25.0f, .swing_arc = 15.0f, .attack_duration = 0.1f},
  {.type = SWORD, .damage = 51, .cooldown = 0.25f, .range = 50.0f, .swing_arc = 90.0f, .attack_duration = 0.25f},
  {.type = HAMMER, .damage = 150, .cooldown = 0.5f, .range = 100.0f, .swing_arc = 180.0f, .attack_duration = 0.25f},
  {.type = GRENADE, .damage = 100, .cooldown = 2.0f, .explosion_radius = 100, .fuse_time = 2.0f, .throw_speed = 500},
  {.type = THROWING_KNIFE, .damage = 100, .cooldown = 2.0f, .throw_speed = 600},
  {.type = LAND_MINE, .damage = 100, .cooldown = 2.0f, .explosion_radius = 100, .fuse_time = 0.1f},
  {.type = SMOKE_GRENADE, .damage = 0, .cooldown = 2.0f, .explosion_radius = 100, .fuse_time = 2.0f, .throw_speed = 300}
};

// functions
void initializePlayer(Player *player) {
  player->position = (Vector2){(float)SCREEN_WIDTH/2.0f, (float)SCREEN_HEIGHT/2.0f};
  player->radius = 5;
  player->health = 100;
  player->armor = 0;
  player->fire_timer = 0.0f;
  player->weapons[0] = weapon_list[AK47];
  player->weapons[1] = weapon_list[GRENADE];
  player->active_weapon = 0;
  player->reload_timer = 0.0f;
  player->is_reloading = false;
  player->is_attacking = false;
  player->attack_timer = 0.0f;
  player->attack_angle = 0.0f;
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

void bounceGrenade(Grenade *grenades, Rectangle rec){
  float left_overlap = (rec.x + rec.width) - (grenades->position.x - 5.0f);
  float right_overlap = (grenades->position.x + 5.0f) - (rec.x);
  float top_overlap = (rec.y + rec.height) - (grenades->position.y - 5.0f);
  float bottom_overlap = (grenades->position.y + 5.0f) - (rec.y);
  // bounces grenade
  float min_overlap = fminf(fminf(left_overlap, right_overlap), fminf(top_overlap, bottom_overlap));
  if(min_overlap == left_overlap){
    grenades->direction.x *= -1;
    grenades->position.x = rec.x + rec.width + 5.0f;
  }
  else if(min_overlap == right_overlap){
    grenades->direction.x *= -1;
    grenades->position.x = rec.x - 5.0f;
  }
  else if(min_overlap == top_overlap){
    grenades->direction.y *= -1;
    grenades->position.y = rec.y + rec.height + 5.0f;
  }
  else if(min_overlap == bottom_overlap){
    grenades->direction.y *= -1;
    grenades->position.y = rec.y - 5.0f;
  }
}

void spawnGrenade(Grenade *grenades, Player *player, Weapon weapon){
  for (int i = 0; i < GRENADE_COUNT; i++){
    if (grenades[i].active == false)
    {
    grenades[i].type = weapon.type;
    grenades[i].position = player->position;
    grenades[i].direction = (Vector2Normalize(
        Vector2Subtract(
          GetMousePosition(), player->position)
        ));
    grenades[i].friction = 0.98f;
    grenades[i].speed = weapon.throw_speed;
    grenades[i].fuse_timer = weapon.fuse_time;
    grenades[i].damage = weapon.damage;
    grenades[i].explosion_radius = weapon.explosion_radius;
    grenades[i].active = true;
    break;
    }
  }
}
void spawnProjectile(Projectile *projectiles, Player *player, Weapon weapon){
  for (int proj = 0; proj < PROJECTILE_COUNT; proj++)
  {
    if (projectiles[proj].active == false)
    {
      player->weapons[player->active_weapon].current_ammo--;
      projectiles[proj].position = player->position;
      projectiles[proj].direction = (Vector2Normalize(
        Vector2Subtract(
          GetMousePosition(), player->position)
        ));
      float spread = (1.0f - weapon.accuracy) * 10.0f;
      float speed = Vector2Length(player->velocity);
      spread += speed * 0.03f;
      float offset = (float)GetRandomValue((int)-spread, (int)spread);
      projectiles[proj].direction = Vector2Rotate(projectiles[proj].direction, offset * DEG2RAD);
      projectiles[proj].speed = 2000;
      projectiles[proj].damage = weapon.damage;
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

void spawnExplosion(Explosion *explosions, Vector2 position, float max_radius){
  for (int i = 0; i < EXPLOSION_COUNT; i++)
  {
    if (explosions[i].active == false)
    {
      explosions[i].position = position;
      explosions[i].max_radius = max_radius;
      explosions[i].radius = 0;
      explosions[i].duration = 0.3f;
      explosions[i].timer = 0.3f;
      explosions[i].active = true;
      break;
    }
  }
}

bool isMeleeWeapon(WeaponType type){
  return type == KNIFE || type == SWORD || type == HAMMER;
}
bool isThrowableWeapon(WeaponType type){
  return type == GRENADE || type == THROWING_KNIFE || type == SMOKE_GRENADE || type == LAND_MINE;
}
// line rec helper for grenade wall collision
bool checkCollisionLineRec(Grenade *grenades, Rectangle rec){
  Vector2 collision_point;
  if (CheckCollisionLines(
      grenades->prev_position, grenades->position,
      (Vector2){rec.x, rec.y}, (Vector2){rec.x + rec.width, rec.y},
      &collision_point)
  || CheckCollisionLines(
      grenades->prev_position, grenades->position, 
      (Vector2){rec.x, rec.y + rec.height}, (Vector2){rec.x + rec.width, rec.y + rec.height},
      &collision_point)
  || CheckCollisionLines(
      grenades->prev_position, grenades->position, 
      (Vector2){rec.x, rec.y}, (Vector2){rec.x, rec.y + rec.height},
      &collision_point)
  || CheckCollisionLines(
      grenades->prev_position, grenades->position, 
      (Vector2){rec.x + rec.width, rec.y}, (Vector2){rec.x + rec.width, rec.y + rec.height},
      &collision_point)){
    return true;
  }
  return false;
}

int main(void) {
  Player p1;
  Player p2;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game window");
  // sound
  InitAudioDevice();
  Sound gunshot = LoadSound("gunshot.wav");
  Sound impact = LoadSound("impact.wav");
  Sound knife_slash = LoadSound("knife_slash.wav");
  Sound pinpull = LoadSound("pinpull.wav");
  Sound explode = LoadSound("explode.wav");
  Sound he_bounce = LoadSound("he_bounce.wav");

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
  int pickup_count = 0;
  for(int rows = 0; rows < ROW_COUNT; rows++){
    for(int columns = 0; columns < COLUMN_COUNT; columns++){
      // adds things to map arrays based on map file 1 = walls, 2 = weapon, etc.
      char map_char = fgetc(map);
        if(map_char == '1'){ 
          walls[char_count] = (Rectangle){
            columns * TILE_WIDTH, rows * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT}; 
          char_count++; 
        }
        else if(map_char == '2'){
          pickups[pickup_count].type = PICKUP_WEAPON;
          pickups[pickup_count].position = (Vector2){
            columns * TILE_WIDTH + TILE_WIDTH/2, 
            rows * TILE_HEIGHT + TILE_HEIGHT/2
          };
          pickups[pickup_count].weapon = (WeaponType)GetRandomValue(0, 10);
          pickups[pickup_count].active = true;
          pickup_count++;
        }
        else if(map_char == '3'){
          pickups[pickup_count].type = PICKUP_HEALTH;
          pickups[pickup_count].position = (Vector2){
            columns * TILE_WIDTH + TILE_WIDTH/2, 
            rows * TILE_HEIGHT + TILE_HEIGHT/2
          };
          pickups[pickup_count].value = 50;
          pickups[pickup_count].active = true;
          pickup_count++;
        }
        else if(map_char == '4'){
          pickups[pickup_count].type = PICKUP_AMMO;
          pickups[pickup_count].position = (Vector2){
            columns * TILE_WIDTH + TILE_WIDTH/2, 
            rows * TILE_HEIGHT + TILE_HEIGHT/2
          };
          pickups[pickup_count].value = 50;
          pickups[pickup_count].active = true;
          pickup_count++;
        }
        else if(map_char == '5'){
           pickups[pickup_count].type = PICKUP_ARMOR;
            pickups[pickup_count].position = (Vector2){
            columns * TILE_WIDTH + TILE_WIDTH/2, 
            rows * TILE_HEIGHT + TILE_HEIGHT/2
          };
          pickups[pickup_count].value = 100;
          pickups[pickup_count].active = true;
          pickup_count++;
        }
    }
    fgetc(map); // consume newline character
  }
  

  while(!WindowShouldClose()) // while the window shouldn't be closing (due to x, alt+f4, etc.)
  {
    // Update variables here:
    Vector2 current_position = p1.position;

    updatePlayer(&p1);
    
    // weapon switch
    if(IsKeyPressed(KEY_Q)){
      p1.active_weapon = (p1.active_weapon == 0) ? 1 : 0;
    }
    p1.fire_timer += GetFrameTime();

    //firing check
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
      if(isMeleeWeapon(p1.weapons[p1.active_weapon].type)){
        if(p1.fire_timer >= p1.weapons[p1.active_weapon].cooldown){
          Vector2 to_mouse = Vector2Subtract(GetMousePosition(), p1.position);
          float aim_angle = atan2f(to_mouse.y, to_mouse.x);
          p1.attack_angle = aim_angle - (p1.weapons[p1.active_weapon].swing_arc / 2 * DEG2RAD);
          p1.is_attacking = true;
          PlaySound(knife_slash);
          p1.fire_timer = 0.0f;
        }
      } 
      else if(isThrowableWeapon(p1.weapons[p1.active_weapon].type)){
        if(p1.fire_timer >= p1.weapons[p1.active_weapon].cooldown){
          spawnGrenade(grenades, &p1, p1.weapons[p1.active_weapon]);
          PlaySound(pinpull);
          p1.fire_timer = 0.0f;
        }
      }
      else {
        if (p1.fire_timer >= p1.weapons[p1.active_weapon].cooldown
          && p1.weapons[p1.active_weapon].current_ammo > 0
          && !p1.is_reloading){
            spawnProjectile(projectiles, &p1, p1.weapons[p1.active_weapon]);
            PlaySound(gunshot);
            p1.fire_timer = 0.0f;
        }
      }
    }
    // melee swing update
    if(p1.is_attacking){
      Weapon *w = &p1.weapons[p1.active_weapon];
      float swing_speed = (w->swing_arc * DEG2RAD) / w->attack_duration;
      p1.attack_angle += swing_speed * GetFrameTime();
      p1.attack_timer += GetFrameTime();
      Vector2 weapon_tip = {
        p1.position.x + cosf(p1.attack_angle) * p1.weapons[p1.active_weapon].range,
        p1.position.y + sinf(p1.attack_angle) * p1.weapons[p1.active_weapon].range
      };
      if(CheckCollisionCircleLine(p2.position, p2.radius, p1.position, weapon_tip)){
        p2.health -= p1.weapons[p1.active_weapon].damage;
      }
      if(p1.attack_timer >= w->attack_duration){
        p1.is_attacking = false;
        p1.attack_timer = 0.0f;
      }
    }
    // reloading check
    if(IsKeyPressed(KEY_R) && !p1.is_reloading){
     p1.is_reloading = true;
      p1.reload_timer = p1.weapons[p1.active_weapon].reload_time;
    }

    if(p1.is_reloading){
    p1.reload_timer -= GetFrameTime();
    if(p1.reload_timer <= 0){
      p1.weapons[p1.active_weapon].current_ammo = p1.weapons[p1.active_weapon].magazine_size;
      p1.is_reloading = false;
    }
  }
    //projectile update loop
    for (int i = 0; i < PROJECTILE_COUNT; i++)
    {
      if(projectiles[i].active == true){
      projectiles[i].prev_position = projectiles[i].position;
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
    // grenade update loop
    for (int i = 0; i < GRENADE_COUNT; i++)
    {
      if(grenades[i].active == true){
        int steps = 5;
        for(int step = 0; step < steps; step++){ 
          grenades[i].prev_position = grenades[i].position;
          grenades[i].position = Vector2Add(grenades[i].position, 
            Vector2Scale(grenades[i].direction, grenades[i].speed * GetFrameTime() / steps)); // substep movement, moves 5 times per frame
          for(int wall = 0; wall < char_count; wall++){
            if(CheckCollisionCircleRec(grenades[i].position, 5.0f, walls[wall])){
              if(grenades[i].type == THROWING_KNIFE){
              grenades[i].active = false;
              } else {
              bounceGrenade(&grenades[i], walls[wall]);
              PlaySound(he_bounce);
              }
            }
          }
        }
        grenades[i].speed *= grenades[i].friction;
        grenades[i].fuse_timer -= GetFrameTime();
        if(grenades[i].type == THROWING_KNIFE){
          if(CheckCollisionCircles(p2.position, p2.radius, grenades[i].position, 5.0f)){
            p2.health -= grenades[i].damage;
            grenades[i].active = false;
          }
        }
        if (grenades[i].fuse_timer <= 0 && grenades[i].type != THROWING_KNIFE){
          if(CheckCollisionCircles(p2.position, p2.radius, grenades[i].position, grenades[i].explosion_radius)){
            p2.health -= grenades[i].damage;
          }
          spawnExplosion(explosions, grenades[i].position, grenades[i].explosion_radius);
          PlaySound(explode);
          grenades[i].active = false;
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
    // projectile player collision check loop
    for (int p = 0; p < PROJECTILE_COUNT; p++){
      if(projectiles[p].active == true &&
      CheckCollisionCircleLine(p2.position, p2.radius, projectiles[p].position, projectiles[p].prev_position) == true){
        p2.health -= projectiles[p].damage; // currently p2 only for testing
        projectiles[p].active = false;
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
    // explosion update loop
    for (int i = 0; i < EXPLOSION_COUNT; i++)
    {
      if(explosions[i].active == true){
        explosions[i].timer -= GetFrameTime();
        explosions[i].radius = explosions[i].max_radius * (1.0f - explosions[i].timer / explosions[i].duration);
        if(explosions[i].timer <= 0){
          explosions[i].active = false;
        }
      }
    }
    // pickup effects
      for(int i = 0; i < pickup_count; i++){
        if(pickups[i].active == true){
          if(CheckCollisionCircles(p1.position, p1.radius, pickups[i].position, 10)){
            if(pickups[i].type == PICKUP_HEALTH){
              p1.health += pickups[i].value;
                if(p1.health > 100) {
                  p1.health = 100;}
            }
            else if(pickups[i].type == PICKUP_WEAPON){
              p1.weapons[p1.active_weapon] = weapon_list[pickups[i].weapon];
            }
            else if(pickups[i].type == PICKUP_AMMO){
              p1.weapons[p1.active_weapon].current_ammo = p1.weapons[p1.active_weapon].magazine_size;
            }
            else if(pickups[i].type == PICKUP_ARMOR){
              p1.armor +=pickups[i].value;
            }
            pickups[i].active = false;
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
      // draws explosions
      for(int i = 0; i < EXPLOSION_COUNT; i++){
        if(explosions[i].active == true){
          float alpha = explosions[i].timer / explosions[i].duration;
          DrawCircleV(explosions[i].position, explosions[i].radius, ColorAlpha(ORANGE, alpha));
        }
      }
      // draws pickups
      for(int i = 0; i < pickup_count; i++){
        if(pickups[i].active == true){
          Color pickup_color;
          if(pickups[i].type == PICKUP_WEAPON) pickup_color = BLUE;
          else if(pickups[i].type == PICKUP_HEALTH) pickup_color = GREEN;
          else if(pickups[i].type == PICKUP_AMMO) pickup_color = YELLOW;
          else if(pickups[i].type == PICKUP_ARMOR) pickup_color = GRAY;
          DrawCircleV(pickups[i].position, 10, pickup_color);
        }
      }
      // draws melee swings
      if(p1.is_attacking){
        Vector2 weapon_tip = {
        p1.position.x + cosf(p1.attack_angle) * p1.weapons[p1.active_weapon].range,
        p1.position.y + sinf(p1.attack_angle) * p1.weapons[p1.active_weapon].range
        };
        DrawLineEx(p1.position, weapon_tip, 3.0f, DARKGRAY);
      }
      // draws grenades
      for(int i = 0; i < GRENADE_COUNT; i++){
        if(grenades[i].active == true){
          DrawCircleV(grenades[i].position, 5, DARKGREEN);
        }
      }
      DrawText(TextFormat("Pos: %.1f, %.1f", p1.position.x, p1.position.y), 20, 20, 20, BLACK);
      DrawText(TextFormat("W:", p1.keys_pressed[W]), -20, 20, 20, BLACK);
      DrawText(TextFormat("P2 Health: %d", p2.health), 90, 120, 20, RED);
      DrawText(TextFormat("Weapon: %d", p1.active_weapon), 90, 260, 20, BLACK);
      DrawText(TextFormat("Ammo: %d / %d", 
      p1.weapons[p1.active_weapon].current_ammo, 
      p1.weapons[p1.active_weapon].magazine_size), 20, 80, 20, BLACK);
      if(p1.is_reloading){
        DrawText("Reloading...", 20, 100, 20, RED);
      }
    EndDrawing();
  }
  UnloadSound(gunshot);
  UnloadSound(impact);
  UnloadSound(knife_slash);
  UnloadSound(pinpull);
  UnloadSound(he_bounce);
  UnloadSound(explode);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}