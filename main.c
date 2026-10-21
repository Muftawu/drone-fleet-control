#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define winWidth 1440
#define winHeight 960
#define FPS 60

const char *winName = "Fleet Course Correction";

typedef struct Drone {
  Rectangle rect;
  Color color;
  Vector2 velocity;
  float rotation;
  Texture2D texture;
} Drone;

typedef struct State {
  const char *pattern;
  int slaveCount;
  Vector2 startingPos;
  int offset;
  Vector2 masterDim;
  Vector2 slaveDim;
  int stateChanged;
  const char *droneImg;
  int running;
  int updateFlight;
} State;

void UpdateVelocity(Vector2 p1, Vector2 p2, float frameTime,
                    Vector2 *velocity) {

  int dx = p2.x - p1.x;
  int dy = p2.y - p1.y;

  float speedX = dx * frameTime;
  float speedY = dy * frameTime;

  velocity->x = speedX;
  velocity->y = speedY;
}

Vector2 *GetVelocities(Vector2 *velocities, Vector2 *newCoords, Drone *slaves,
                       int slaveCount, float frameTime) {

  for (int i = 0; i < slaveCount; i++) {
    int dx = slaves[i].rect.x - newCoords[i].x;
    int dy = slaves[i].rect.y - newCoords[i].y;

    float vx = dx * frameTime;
    float vy = dy * frameTime;

    velocities[i] = (Vector2){.x = dx, .y = dy};
  }
  return velocities;
}

void GetNewCoords(Vector2 *coords, Drone *slaves, State state) {
  int slaveCount = state.slaveCount;

  if (strcmp(state.pattern, "Linear") == 0) {

    for (int i = 0; i < slaveCount; i++) {
      int xpos = slaves[i].rect.x;
      int ypos = slaves[i].rect.y + state.offset - state.slaveDim.y;

      Vector2 icoord = (Vector2){.x = xpos, .y = ypos};
      coords[i] = icoord;
    }
  } else if (strcmp(state.pattern, "Assembly") == 0) {

    int assemblyLineCount = (int)slaveCount / 2;

    // assembly line 1
    for (int j = 0; j < assemblyLineCount; j++) {

      int xpos = slaves[j].rect.x;
      int ypos = slaves[j].rect.y - state.offset;

      Vector2 jcoord = (Vector2){.x = xpos, .y = ypos};
      coords[j] = jcoord;
    }

    // assembly line 2
    for (int k = 0; k < slaveCount - assemblyLineCount; k++) {

      int xpos = slaves[k].rect.x + state.slaveDim.x - state.offset;
      int ypos = slaves[k].rect.y + state.slaveDim.y + state.offset;

      Vector2 kcoord = (Vector2){.x = xpos, .y = ypos};
      coords[k + assemblyLineCount] = kcoord;
    }
  }
}

int main() {

  InitWindow(winWidth, winHeight, winName);
  SetTargetFPS(FPS);

  State state = (State){
      .pattern = "Linear",
      .offset = 100,
      .slaveCount = 4,
      .startingPos = (Vector2){.x = 0.4 * winWidth, .y = 0.5 * winHeight},
      .masterDim = (Vector2){.x = 100, .y = 70},
      .slaveDim = (Vector2){.x = 100, .y = 70},
      .stateChanged = 0,
      .droneImg = "./assets/drone.png",
      .updateFlight = 0,
  };

  Drone *slaves = malloc(sizeof(Drone) * state.slaveCount);
  // Vector2 *coords = malloc(sizeof(Vector2) * state.slaveCount);
  Vector2 *newCoords = malloc(sizeof(Vector2) * state.slaveCount);

  Vector2 *velocities = malloc(sizeof(Vector2) * state.slaveCount);

  Image droneImg = LoadImage(state.droneImg);
  ImageResize(&droneImg, state.masterDim.x, state.masterDim.y);
  Texture droneImgTexture = LoadTextureFromImage(droneImg);

  Drone master = (Drone){
      .rect = (Rectangle){.x = 1.3 * state.startingPos.x,
                          .y = state.startingPos.y,
                          .width = state.masterDim.x,
                          .height = state.masterDim.y},
      .color = RAYWHITE,
      .rotation = 0.0f,
      .velocity = (Vector2){.x = 1, .y = 0},
      .texture = (Texture2D)droneImgTexture,
  };

  for (int i = 0; i < state.slaveCount; i++) {
    slaves[i] = (Drone){
        .rect = (Rectangle){.x = state.startingPos.x - i * state.offset -
                                 (i * 0.5 * state.offset),
                            .y = state.startingPos.y,
                            .width = state.slaveDim.x,
                            .height = state.slaveDim.y},
        .color = BLUE,
        .rotation = 0.0f,
        .velocity = (Vector2){.x = 0, .y = 0},
        .texture = (Texture2D)droneImgTexture,
    };
  }

  for (int i = 0; i < state.slaveCount; i++) {
    newCoords[i] = (Vector2){.x = slaves[i].rect.x, .y = slaves[i].rect.y};
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    float frameTime = GetFrameTime();

    if (IsKeyPressed(KEY_R)) {
      master.rect.x = state.startingPos.x + 1.4 * state.offset;
      master.rect.y = state.startingPos.y;

      for (int i = 0; i < state.slaveCount; i++) {
        slaves[i].rect.x = state.startingPos.x - 1.5 * i * state.offset;
        slaves[i].rect.y = state.startingPos.y;
      }
    }

    if (IsKeyPressed(KEY_SPACE)) {
      state.running = ~state.running;
      for (int i = 0; i < state.slaveCount; i++) {
        DrawCircle(slaves[i].rect.x, slaves[i].rect.y, 10, RED);
      }
    }

    if (IsKeyPressed(KEY_A)) {
      if (strcmp(state.pattern, "Assembly") != 0) {
        state.pattern = "Assembly";
        state.stateChanged = 1;
      } else {
        printf("NOOP\n");
      }
    }

    if (IsKeyPressed(KEY_L)) {
      if (strcmp(state.pattern, "Linear") != 0) {
        state.pattern = "Linear";
        state.stateChanged = 1;
      } else {
        printf("NOOP\n");
      }
    }

    if (state.stateChanged) {
      GetNewCoords(newCoords, slaves, state);

      // for (int i = 0; i < state.slaveCount; i++) {
      //   UpdateVelocity((Vector2){.x = slaves[i].rect.x, .y =
      //   slaves[i].rect.y},
      //                  newCoords[i], frameTime, &slaves[i].velocity);
      // }

      GetVelocities(velocities, newCoords, slaves, state.slaveCount, frameTime);
      state.updateFlight = 1;
    }

    // NOTE: trying constant velocity update
    // and have velocity move between 0 and the newly calcultated

    // if (state.updateFlight) {
    //   for (int i = 0; i < state.slaveCount; i++) {
    //     slaves[i].rect.x += velocities[i].x;
    //     slaves[i].rect.y += velocities[i].y;
    //   }
    //
    //   state.updateFlight = 0;
    // }

    if (state.running) {
      master.rect.x += master.velocity.x;
      master.rect.y += master.velocity.y;

      for (int i = 0; i < state.slaveCount; i++) {
        slaves[i].rect.x += master.velocity.x;
      }
    }

    for (int i = 0; i < state.slaveCount; i++) {
      DrawCircle(newCoords[i].x, newCoords[i].y, 10, YELLOW);
      DrawText(TextFormat("%d", i), newCoords[i].x, newCoords[i].y - 30, 20,
               RAYWHITE);
    }

    DrawFPS(50, 50);

    DrawTexture(master.texture, master.rect.x, master.rect.y, master.color);

    for (int i = 0; i < state.slaveCount; i++) {
      DrawText(TextFormat("%d", i), slaves[i].rect.x,
               slaves[i].rect.y + 0.7 * state.offset, 30, RAYWHITE);
      DrawTexture(slaves[i].texture, slaves[i].rect.x, slaves[i].rect.y,
                  slaves[i].color);
    }

    EndDrawing();
  }

  CloseWindow();

  free(slaves);
  free(newCoords);
  free(velocities);

  return 0;
}
