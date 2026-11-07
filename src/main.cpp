#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int WORLD_X = 48;
constexpr int WORLD_Y = 24;
constexpr int WORLD_Z = 48;
constexpr int CHUNK_SIZE = 16;
constexpr float REACH = 6.0f;

enum Block : unsigned char { AIR = 0, GRASS, DIRT, STONE, SAND, WOOD, LEAVES, BLOCK_COUNT };
struct Cell { int x, y, z; };

Block world[WORLD_X][WORLD_Y][WORLD_Z]{};
Texture2D textures[BLOCK_COUNT]{};

bool InBounds(int x, int y, int z) {
    return x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y && z >= 0 && z < WORLD_Z;
}

Color BaseColor(Block b) {
    switch (b) {
        case GRASS: return {104, 157, 62, 255};
        case DIRT: return {125, 83, 49, 255};
        case STONE: return {126, 126, 126, 255};
        case SAND: return {218, 203, 143, 255};
        case WOOD: return {135, 96, 55, 255};
        case LEAVES: return {65, 125, 48, 255};
        default: return MAGENTA;
    }
}

// Make small pixel-art textures locally so the prototype needs no downloaded assets.
Texture2D MakePixelTexture(Block block) {
    Image image = GenImageColor(16, 16, BaseColor(block));
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            const unsigned int noise = static_cast<unsigned int>(
                (x * 92837111u + y * 689287499u + static_cast<unsigned int>(block) * 283923481u) >> 8);
            const int shade = static_cast<int>(noise % 31u) - 15;
            Color c = BaseColor(block);
            c.r = static_cast<unsigned char>(std::clamp(static_cast<int>(c.r) + shade, 0, 255));
            c.g = static_cast<unsigned char>(std::clamp(static_cast<int>(c.g) + shade, 0, 255));
            c.b = static_cast<unsigned char>(std::clamp(static_cast<int>(c.b) + shade, 0, 255));
            if (block == WOOD && (x == 3 || x == 11)) {
                c = {94, 63, 38, 255};
            }
            if (block == LEAVES && ((x + y) % 7 == 0)) {
                c = {84, 145, 57, 255};
            }
            ImageDrawPixel(&image, x, y, c);
        }
    }
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    return texture;
}

void CreateTextures() {
    for (int i = GRASS; i < BLOCK_COUNT; ++i) textures[i] = MakePixelTexture(static_cast<Block>(i));
}

void GenerateTree(int x, int z) {
    int ground = -1;
    for (int y = WORLD_Y - 1; y >= 0; --y) {
        if (world[x][y][z] != AIR) { ground = y; break; }
    }
    if (ground < 0 || ground + 6 >= WORLD_Y) return;
    for (int y = ground + 1; y <= ground + 4; ++y) world[x][y][z] = WOOD;
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dz = -2; dz <= 2; ++dz) {
            for (int dy = 2; dy <= 5; ++dy) {
                const int x2 = x + dx, y2 = ground + dy, z2 = z + dz;
                if (!InBounds(x2, y2, z2)) continue;
                if (std::abs(dx) + std::abs(dz) + (dy == 5 ? 2 : 0) <= 4 &&
                    world[x2][y2][z2] == AIR) world[x2][y2][z2] = LEAVES;
            }
        }
    }
}

void GenerateWorld() {
    for (int x = 0; x < WORLD_X; ++x) {
        for (int z = 0; z < WORLD_Z; ++z) {
            const float broad = std::sin(x * 0.21f) * 1.7f + std::cos(z * 0.19f) * 1.5f
                              + std::sin((x + z) * 0.11f);
            const float detail = std::sin(x * 0.73f + z * 1.17f) * 0.65f
                               + std::cos(z * 0.61f - x * 0.37f) * 0.45f;
            const int height = std::clamp(7 + static_cast<int>(broad + detail), 3, 13);
            for (int y = 0; y <= height; ++y) {
                if (y == height) world[x][y][z] = height <= 5 ? SAND : GRASS;
                else if (y >= height - 3) world[x][y][z] = DIRT;
                else world[x][y][z] = STONE;
            }
        }
    }
    GenerateTree(8, 8);
    GenerateTree(22, 13);
    GenerateTree(34, 31);
    GenerateTree(12, 37);
    GenerateTree(5, 25);
    GenerateTree(27, 39);
    GenerateTree(41, 12);
}

bool FindTarget(const Ray& ray, Cell& target, Vector3& normal) {
    float closest = REACH;
    bool found = false;
    for (int x = 0; x < WORLD_X; ++x) {
        for (int y = 0; y < WORLD_Y; ++y) {
            for (int z = 0; z < WORLD_Z; ++z) {
                if (world[x][y][z] == AIR) continue;
                BoundingBox box = {
                    {static_cast<float>(x) - 0.5f, static_cast<float>(y) - 0.5f, static_cast<float>(z) - 0.5f},
                    {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f}
                };
                RayCollision hit = GetRayCollisionBox(ray, box);
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

bool IsSolidAt(int x, int y, int z) {
    return InBounds(x, y, z) && world[x][y][z] != AIR;
}

bool HasVisibleFace(int x, int y, int z) {
    // A block completely surrounded by other blocks cannot be seen, so skip it.
    return !IsSolidAt(x + 1, y, z) || !IsSolidAt(x - 1, y, z) ||
           !IsSolidAt(x, y + 1, z) || !IsSolidAt(x, y - 1, z) ||
           !IsSolidAt(x, y, z + 1) || !IsSolidAt(x, y, z - 1);
}

Color MixColor(Color a, Color b, float amount) {
    amount = std::clamp(amount, 0.0f, 1.0f);
    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * amount),
        static_cast<unsigned char>(a.g + (b.g - a.g) * amount),
        static_cast<unsigned char>(a.b + (b.b - a.b) * amount),
        255
    };
}

Color GetBlockTint(int x, int y, int z, const Camera3D& camera) {
    // A lightweight block-level lighting approximation: higher terrain is brighter.
    int brightness = std::clamp(218 + (y - 6) * 4, 185, 255);
    if (!IsSolidAt(x, y + 1, z)) brightness = std::min(255, brightness + 12);
    Color tint = {static_cast<unsigned char>(brightness),
                  static_cast<unsigned char>(brightness),
                  static_cast<unsigned char>(brightness), 255};

    // Blend distant blocks toward the sky color to create atmospheric distance fog.
    const Vector3 center = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
    const float distance = Vector3Distance(camera.position, center);
    const float fog = std::clamp((distance - 18.0f) / 34.0f, 0.0f, 0.82f);
    return MixColor(tint, {151, 198, 232, 255}, fog);
}

void DrawWorld(const Camera3D& camera) {
    // Group traversal by 16x16 regions and omit fully hidden blocks.
    // This reduces draw calls for enclosed terrain while preserving the simple prototype renderer.
    for (int cx = 0; cx < WORLD_X; cx += CHUNK_SIZE) {
        for (int cz = 0; cz < WORLD_Z; cz += CHUNK_SIZE) {
            for (int x = cx; x < std::min(cx + CHUNK_SIZE, WORLD_X); ++x) {
                for (int z = cz; z < std::min(cz + CHUNK_SIZE, WORLD_Z); ++z) {
                    for (int y = 0; y < WORLD_Y; ++y) {
                        const Block b = world[x][y][z];
                        if (b == AIR || !HasVisibleFace(x, y, z)) continue;
                        DrawCubeTexture(textures[b],
                            {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)},
                            1.0f, 1.0f, 1.0f, GetBlockTint(x, y, z, camera));
                    }
                }
            }
        }
    }
}
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "LimeCraft Alpha 0.3");
    SetTargetFPS(60);
    DisableCursor();

    CreateTextures();
    GenerateWorld();

    Camera3D camera{};
    camera.position = {24.0f, 15.0f, 34.0f};
    camera.target = {24.0f, 14.5f, 33.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const Block hotbar[] = {GRASS, DIRT, STONE, SAND, WOOD};
    Block selected = GRASS;
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

        Ray ray = GetMouseRay({GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f}, camera);
        hasTarget = FindTarget(ray, aimed, faceNormal);
        if (hasTarget && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            world[aimed.x][aimed.y][aimed.z] = AIR;
        }
        if (hasTarget && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            const int px = aimed.x + static_cast<int>(std::round(faceNormal.x));
            const int py = aimed.y + static_cast<int>(std::round(faceNormal.y));
            const int pz = aimed.z + static_cast<int>(std::round(faceNormal.z));
            if (InBounds(px, py, pz) && world[px][py][pz] == AIR) world[px][py][pz] = selected;
        }

        BeginDrawing();
        ClearBackground({151, 198, 232, 255});
        BeginMode3D(camera);
        DrawWorld(camera);
        EndMode3D();

        const int cx = GetScreenWidth() / 2, cy = GetScreenHeight() / 2;
        DrawLine(cx - 7, cy, cx + 7, cy, WHITE);
        DrawLine(cx, cy - 7, cx, cy + 7, WHITE);

        DrawRectangle(12, 12, 405, 88, Fade(BLACK, 0.55f));
        DrawText("LIMECRAFT  |  ALPHA 0.3", 24, 20, 21, LIME);
        DrawText("WASD: move   SPACE: jump   Mouse: look", 24, 50, 14, RAYWHITE);
        DrawText("LMB: break   RMB: place   1-5: select", 24, 70, 14, RAYWHITE);
        DrawText("Terrain detail | distance fog | hidden-block culling", 24, 88, 12, SKYBLUE);

        const int slot = 48, gap = 6;
        const int startX = GetScreenWidth() / 2 - (5 * slot + 4 * gap) / 2;
        const int barY = GetScreenHeight() - 66;
        for (int i = 0; i < 5; ++i) {
            const int x = startX + i * (slot + gap);
            DrawRectangle(x, barY, slot, slot, Fade(DARKGRAY, 0.92f));
            DrawTextureEx(textures[hotbar[i]], {static_cast<float>(x + 7), static_cast<float>(barY + 7)},
                          0.0f, 2.1f, WHITE);
            DrawRectangleLines(x, barY, slot, slot, selected == hotbar[i] ? YELLOW : LIGHTGRAY);
        }
        DrawFPS(GetScreenWidth() - 90, 12);
        EndDrawing();
    }

    for (int i = GRASS; i < BLOCK_COUNT; ++i) UnloadTexture(textures[i]);
    EnableCursor();
    CloseWindow();
    return 0;
}
