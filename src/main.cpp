#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr int WORLD_X = 32;
constexpr int WORLD_Y = 16;
constexpr int WORLD_Z = 32;
constexpr float REACH = 6.0f;

enum Block : unsigned char {
    AIR = 0,
    GRASS,
    DIRT,
    STONE,
    SAND,
    WOOD
};

struct Cell {
    int x;
    int y;
    int z;
};

Block world[WORLD_X][WORLD_Y][WORLD_Z]{};

bool InBounds(int x, int y, int z) {
    return x >= 0 && x < WORLD_X &&
           y >= 0 && y < WORLD_Y &&
           z >= 0 && z < WORLD_Z;
}

Color BlockColor(Block block) {
    switch (block) {
        case GRASS: return Color{92, 150, 58, 255};
        case DIRT:  return Color{126, 86, 55, 255};
        case STONE: return Color{125, 125, 125, 255};
        case SAND:  return Color{218, 204, 143, 255};
        case WOOD:  return Color{116, 82, 48, 255};
        default:    return BLANK;
    }
}

void GenerateWorld() {
    for (int x = 0; x < WORLD_X; ++x) {
        for (int z = 0; z < WORLD_Z; ++z) {
            const float wave =
                std::sin(static_cast<float>(x) * 0.32f) * 1.3f +
                std::cos(static_cast<float>(z) * 0.27f) * 1.1f;
            const int height = std::clamp(5 + static_cast<int>(wave), 3, 8);

            for (int y = 0; y <= height; ++y) {
                if (y == height) {
                    world[x][y][z] = (height <= 4) ? SAND : GRASS;
                } else if (y >= height - 2) {
                    world[x][y][z] = DIRT;
                } else {
                    world[x][y][z] = STONE;
                }
            }
        }
    }

    // A small tree gives the prototype a bit of classic-world character.
    const int tx = 8;
    const int tz = 8;
    int ground = 0;
    for (int y = WORLD_Y - 1; y >= 0; --y) {
        if (world[tx][y][tz] != AIR) {
            ground = y;
            break;
        }
    }
    for (int y = ground + 1; y <= ground + 4 && y < WORLD_Y; ++y) {
        world[tx][y][tz] = WOOD;
    }
}

bool FindTarget(const Ray& ray, Cell& target, Vector3& normal) {
    float closest = REACH;
    bool found = false;

    for (int x = 0; x < WORLD_X; ++x) {
        for (int y = 0; y < WORLD_Y; ++y) {
            for (int z = 0; z < WORLD_Z; ++z) {
                if (world[x][y][z] == AIR) continue;

                const BoundingBox box = {
                    Vector3{static_cast<float>(x) - 0.5f,
                            static_cast<float>(y) - 0.5f,
                            static_cast<float>(z) - 0.5f},
                    Vector3{static_cast<float>(x) + 0.5f,
                            static_cast<float>(y) + 0.5f,
                            static_cast<float>(z) + 0.5f}
                };

                const RayCollision hit = GetRayCollisionBox(ray, box);
                if (hit.hit && hit.distance < closest) {
                    closest = hit.distance;
                    target = {x, y, z};
                    normal = hit.normal;
                    found = true;
                }
            }
        }
    }
    return found;
}
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "LimeCraft Alpha 0.1");
    SetTargetFPS(60);
    DisableCursor();

    GenerateWorld();

    Camera3D camera{};
    camera.position = Vector3{16.0f, 10.0f, 22.0f};
    camera.target = Vector3{16.0f, 9.5f, 21.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Block selected = GRASS;
    const Block hotbar[] = {GRASS, DIRT, STONE, SAND, WOOD};
    Cell aimed{};
    Vector3 faceNormal{};
    bool hasTarget = false;

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        if (IsKeyPressed(KEY_ONE)) selected = hotbar[0];
        if (IsKeyPressed(KEY_TWO)) selected = hotbar[1];
        if (IsKeyPressed(KEY_THREE)) selected = hotbar[2];
        if (IsKeyPressed(KEY_FOUR)) selected = hotbar[3];
        if (IsKeyPressed(KEY_FIVE)) selected = hotbar[4];

        const Ray ray = GetMouseRay(
            Vector2{static_cast<float>(GetScreenWidth()) * 0.5f,
                    static_cast<float>(GetScreenHeight()) * 0.5f},
            camera
        );
        hasTarget = FindTarget(ray, aimed, faceNormal);

        if (hasTarget && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            world[aimed.x][aimed.y][aimed.z] = AIR;
        }

        if (hasTarget && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const int px = aimed.x + static_cast<int>(std::round(faceNormal.x));
            const int py = aimed.y + static_cast<int>(std::round(faceNormal.y));
            const int pz = aimed.z + static_cast<int>(std::round(faceNormal.z));

            if (InBounds(px, py, pz) && world[px][py][pz] == AIR) {
                world[px][py][pz] = selected;
            }
        }

        BeginDrawing();
        ClearBackground(Color{126, 180, 224, 255});

        BeginMode3D(camera);
        DrawPlane(Vector3{16.0f, 0.0f, 16.0f},
                  Vector2{80.0f, 80.0f}, Color{90, 145, 190, 255});

        for (int x = 0; x < WORLD_X; ++x) {
            for (int y = 0; y < WORLD_Y; ++y) {
                for (int z = 0; z < WORLD_Z; ++z) {
                    const Block block = world[x][y][z];
                    if (block == AIR) continue;
                    const Vector3 position = {
                        static_cast<float>(x),
                        static_cast<float>(y),
                        static_cast<float>(z)
                    };
                    DrawCube(position, 1.0f, 1.0f, 1.0f, BlockColor(block));
                }
            }
        }

        EndMode3D();

        DrawLine(GetScreenWidth() / 2 - 7, GetScreenHeight() / 2,
                 GetScreenWidth() / 2 + 7, GetScreenHeight() / 2, WHITE);
        DrawLine(GetScreenWidth() / 2, GetScreenHeight() / 2 - 7,
                 GetScreenWidth() / 2, GetScreenHeight() / 2 + 7, WHITE);

        DrawRectangle(12, 12, 360, 74, Fade(BLACK, 0.55f));
        DrawText("LIMECRAFT  |  ALPHA 0.1", 24, 20, 20, LIME);
        DrawText("WASD: move   SPACE: jump   Mouse: look", 24, 47, 14, RAYWHITE);
        DrawText("LMB: break   RMB: place   1-5: select block",
                 24, 65, 14, RAYWHITE);

        const int slotSize = 48;
        const int startX = GetScreenWidth() / 2 - (5 * slotSize + 4 * 6) / 2;
        const int barY = GetScreenHeight() - 66;
        for (int i = 0; i < 5; ++i) {
            const int x = startX + i * (slotSize + 6);
            DrawRectangle(x, barY, slotSize, slotSize, Fade(DARKGRAY, 0.9f));
            DrawRectangle(x + 7, barY + 7, slotSize - 14, slotSize - 14,
                          BlockColor(hotbar[i]));
            DrawRectangleLines(x, barY, slotSize, slotSize,
                              selected == hotbar[i] ? YELLOW : LIGHTGRAY);
        }

        DrawFPS(GetScreenWidth() - 90, 12);
        EndDrawing();
    }

    EnableCursor();
    CloseWindow();
    return 0;
}
