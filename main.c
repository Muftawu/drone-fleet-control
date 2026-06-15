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
  const char *masterImg;
  const char *slaveImg;
  int running;
} State;

void UpdateVelocityPro(Vector2 p1, Vector2 p2, float frameTime,
                       Vector2 *velocity) {

  float eps = 0.02;
  int dx = p2.x - p1.x;
  int dy = p2.y - p1.y;

  float speedX = dx * frameTime;
  float speedY = dy * frameTime;

  velocity->x = speedX;
  velocity->y = speedY;
}

void GetFlightPatternCoords(State state, Vector2 *coords) {
  int slaveCount = state.slaveCount;
  int startingPosY = state.startingPos.y;
  int startingPosX = state.startingPos.x;

  if (strcmp(state.pattern, "Linear") == 0) {
    for (int i = 0; i < slaveCount; i++) {
      int xpos = startingPosX - 1.5 * i * state.offset;
      int ypos = startingPosY;
      Vector2 icoord = (Vector2){.x = xpos, .y = ypos};
      coords[i] = icoord;
    }
  } else if (strcmp(state.pattern, "Assembly") == 0) {

    int assemblyLineCount = (int)slaveCount / 2;

    // assembly line 1
    for (int i = 0; i < assemblyLineCount; i++) {
      int xpos = (startingPosX) - (2 * i * state.offset);
      int ypos = startingPosY - state.offset;
      Vector2 icoord = (Vector2){.x = xpos, .y = ypos};
      coords[i] = icoord;
    }

    // assembly line 2
    for (int j = 0; j < slaveCount - assemblyLineCount; j++) {
      int xpos = (startingPosX) - (2 * j * state.offset);
      int ypos = startingPosY + 1.5 * state.offset;
      Vector2 jcoord = (Vector2){.x = xpos, .y = ypos};
      coords[j + assemblyLineCount] = jcoord;
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
      .masterImg = "./assets/drone.png",
      .slaveImg = "./assets/drone.png",
  };

  Drone *slaves = malloc(sizeof(Drone) * state.slaveCount);
  Vector2 *coords = malloc(sizeof(Vector2) * state.slaveCount);
  Vector2 *newCoords = malloc(sizeof(Vector2) * state.slaveCount);

  Image masterImg = LoadImage(state.slaveImg);
  Image slaveImg = LoadImage(state.masterImg);

  ImageResize(&masterImg, state.masterDim.x, state.masterDim.y);
  ImageResize(&slaveImg, state.slaveDim.x, state.slaveDim.y);

  Texture masterImgTexture = LoadTextureFromImage(masterImg);
  Texture slaveImgTexture = LoadTextureFromImage(slaveImg);

  Drone master = (Drone){
      .rect = (Rectangle){.x = 1.3 * state.startingPos.x,
                          .y = state.startingPos.y,
                          .width = state.masterDim.x,
                          .height = state.masterDim.y},
      .color = RAYWHITE,
      .rotation = 0.0f,
      .velocity = (Vector2){.x = 1, .y = 0},
      .texture = (Texture2D)slaveImgTexture,
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
        .texture = (Texture2D)slaveImgTexture,
    };
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    float frameTime = GetFrameTime();

    if (IsKeyPressed(KEY_R)) {
      master.rect.x = state.startingPos.x + 1.4 * state.offset;
      master.rect.y = state.startingPos.y;
    }

    if (IsKeyPressed(KEY_SPACE)) {
      state.running = ~state.running;
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
      GetFlightPatternCoords(state, newCoords);

      // update coords
      for (int i = 0; i < state.slaveCount; i++) {
        UpdateVelocityPro(
            (Vector2){.x = slaves[i].rect.x, .y = slaves[i].rect.y},
            newCoords[i], frameTime, &slaves[i].velocity);
      }

      // apply motion
      for (int i = 0; i < state.slaveCount; i++) {
        slaves[i].rect.x += slaves[i].velocity.x;
        slaves[i].rect.y += slaves[i].velocity.y;
      }
    }

    if (state.running) {
      master.rect.x += master.velocity.x;
      master.rect.y += master.velocity.y;
    }

    for (int i = 0; i < state.slaveCount; i++) {
      DrawCircle(newCoords[i].x, newCoords[i].y, 10, YELLOW);
    }

    DrawFPS(50, 50);

    DrawTexture(master.texture, master.rect.x, master.rect.y, master.color);

    for (int i = 0; i < state.slaveCount; i++) {
      DrawTexture(slaves[i].texture, slaves[i].rect.x, slaves[i].rect.y,
                  slaves[i].color);
    }

    EndDrawing();
  }

  CloseWindow();

  free(slaves);
  free(coords);
  free(newCoords);

  return 0;
}
