# LimeCraft

A small C++ voxel sandbox inspired by the look and feel of classic Minecraft Java 1.7-era gameplay. LimeCraft is an independent project and does not use Minecraft's original source code or assets.

## Alpha 0.3

- First-person camera and movement
- More detailed procedurally generated rolling terrain
- Pixel-style 16x16 block textures generated in C++
- Grass, dirt, stone, sand, wood, and leaf blocks
- More generated trees
- Atmospheric distance-fog effect that blends far blocks toward the sky color
- Basic height-based block brightness
- Hidden-block culling: blocks fully surrounded on all six sides are skipped
- Terrain traversal organized into 16x16 regions
- Break and place blocks
- Number-key hotbar selection

**Renderer note:** Alpha 0.3 uses visible-block culling and distance-based tinting as lightweight improvements. It does not yet have merged chunk meshes or true dynamic shadows.

## Technology

- C++17
- [raylib 5.5](https://www.raylib.com/)
- CMake

## Build

You need a C++17 compiler, CMake 3.20+, and Git. CMake downloads raylib during configuration.

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run the resulting `limecraft` executable from the build output directory.

## Controls

| Input | Action |
|---|---|
| WASD | Move |
| Mouse | Look around |
| Space | Jump |
| Left mouse | Break block |
| Right mouse | Place block |
| 1–5 | Select hotbar block |
| Esc | Quit |

## Roadmap

- [ ] Real chunk mesh batching and visibility culling
- [ ] Classic sky, fog, and lighting
- [ ] Inventory and crafting
- [ ] Save and load worlds
- [ ] Android touch controls and APK build

LimeCraft is a fan-made, independent voxel sandbox project.
