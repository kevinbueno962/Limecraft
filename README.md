# LimeCraft

A small C++ voxel sandbox inspired by the look and feel of classic Minecraft Java 1.7-era gameplay. LimeCraft is an independent project and does not use Minecraft's original source code or assets.

## Alpha 0.1 prototype

- First-person camera and movement
- Procedurally generated block terrain
- Grass, dirt, stone, sand, and wood blocks
- Break and place blocks
- Number-key hotbar selection

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

- [ ] Better block textures and classic lighting
- [ ] Chunk-based world rendering
- [ ] Inventory and crafting
- [ ] Trees, caves, and more terrain features
- [ ] Save and load worlds
- [ ] Android touch controls and APK build

LimeCraft is a fan-made, independent voxel sandbox project.
