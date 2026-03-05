# 🌍 Biome Survivor

**A 3D open-world survival game with 7 unique biomes, deep crafting, base building, and co-op multiplayer.**

---

## Table of Contents

- [About the Game](#about-the-game)
- [Features](#features)
- [Biomes](#biomes)
- [Screenshots](#screenshots)
- [Getting Started (Development)](#getting-started-development)
  - [Prerequisites](#prerequisites)
  - [Setup Instructions](#setup-instructions)
  - [Building the Project](#building-the-project)
- [Project Structure](#project-structure)
- [Development Workflow](#development-workflow)
- [Asset Strategy](#asset-strategy)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

---

## About the Game

**Biome Survivor** is a survival game inspired by *The Long Dark*, *Valheim*, and *Rust* — but set across **7 vastly different biomes**, from frozen arctic tundra to scorching desert wastelands.

Players must manage hunger, thirst, temperature, and health while gathering resources, crafting tools, building shelters, and fighting dangerous wildlife. Play alone in singleplayer, or survive together with friends over LAN or online co-op (up to 32 players).

| Detail | Info |
|--------|------|
| **Engine** | Unreal Engine 5.4+ |
| **Language** | C++ / Blueprints |
| **Platform** | PC (Windows / Linux server) |
| **Players** | 1 (solo) · 2-8 (LAN) · 2-32 (online) |
| **Perspective** | Third-person / First-person (toggle) |
| **Status** | 🔴 Pre-Production |

---

## Features

### Core Gameplay
- **Survival Stats** — Health, hunger, thirst, temperature, stamina, comfort
- **Status Effects** — Hypothermia, heatstroke, bleeding, fractures, poison, infection
- **Day/Night Cycle** — 24-minute real-time days with dynamic lighting
- **Dynamic Weather** — Rain, snow, blizzards, sandstorms, fog, thunderstorms, wildfires

### Movement
- Walk, run, sprint (stamina-based)
- Crouch, prone, swim, climb
- Dodge roll with i-frames
- Momentum-based slope sliding
- Mount riding (horses, camels)
- Boats (raft, canoe, sailboat)

### Inventory
- Grid-based inventory (10×6, expandable)
- Equipment slots (head, torso, legs, feet, hands, back, weapons, tool belt)
- Drag & drop with tooltips, stacking, sorting, filtering
- Item durability and weight system
- Container storage (chests, lockers)

### Crafting
- 7 crafting station types (personal, campfire, workbench, forge, tanning rack, alchemy, sewing)
- 30+ recipes at launch, expandable via data tables
- Progressive recipe discovery
- Item quality and durability system
- Repair and maintenance

### Building
- Snap-based + freeform placement
- 3 material tiers: Wood → Stone → Metal
- Upgradeable structures
- Functional buildings: beds, storage, farms, defenses, power
- Late-game electrical power system

### Combat
- Melee: light/heavy attacks, combos, block, parry, dodge
- Ranged: bow, throwables, firearms (scarce)
- Stealth system with detection model
- 25+ wildlife species with behavior-tree AI
- Hunting, tracking, trapping, and harvesting

### Multiplayer
- Seamless singleplayer ↔ multiplayer
- LAN support (2-8 players, auto-discovery)
- Online co-op (2-32 players, dedicated servers)
- PvE and PvPvE modes
- Team/clan system, trading, proximity voice chat

### Progression
- Use-based skill system (gathering, crafting, combat, survival, stealth, building)
- Achievement and journal system
- Tutorial quests for new players

---

## Biomes

| # | Biome | Temperature | Threat | Key Feature |
|---|-------|-------------|--------|-------------|
| 1 | **Temperate Woods** | 8-18°C | ⭐ Low | Starting biome, balanced resources |
| 2 | **Arctic Tundra** | -28 to -12°C | ⭐⭐⭐⭐ High | Extreme cold, blizzards, polar bears |
| 3 | **Desert** | 8-45°C (day/night swing) | ⭐⭐⭐⭐ High | Water scarcity, sandstorms, extreme heat |
| 4 | **Jungle** | 24-32°C | ⭐⭐⭐ Med-High | Dense vegetation, disease, rare herbs |
| 5 | **Mountain** | -8 to 5°C | ⭐⭐⭐ Medium | Rich ore deposits, avalanches, cliffs |
| 6 | **Swamp** | 16-22°C | ⭐⭐⭐ Medium | Disease, difficult terrain, unique alchemy |
| 7 | **Savanna** | 18-30°C | ⭐⭐⭐ Medium | Large animal herds, wildfires, open terrain |

Each biome has unique wildlife, resources, weather patterns, hazards, and survival challenges. Transition zones blend seamlessly between biomes.

---

## Screenshots

> *Screenshots will be added once the first vertical slice is complete (Month 6).*

---

## Getting Started (Development)

### Prerequisites

Before you begin, ensure you have the following installed. See [requirements.md](requirements.md) for download links and detailed instructions.

| Software | Version | Required |
|----------|---------|----------|
| Windows 11 | Professional 64-bit | ✅ Yes |
| Unreal Engine | 5.4+ | ✅ Yes |
| Visual Studio | 2022 Community+ | ✅ Yes |
| Git | Latest + Git LFS | ✅ Yes |
| Blender | 4.0+ | 🔶 For art work |
| GIMP | 2.10+ | 🔶 For texture work |
| FMOD Studio | Latest | 🔶 For audio work |
| Audacity | Latest | 🔶 For audio editing |

### Setup Instructions

#### 1. Clone the Repository
```bash
git clone https://github.com/YOUR_USERNAME/biome-survivor.git
cd biome-survivor
git lfs pull
```

#### 2. Install Unreal Engine 5.4+
1. Download and install the **Epic Games Launcher** from https://www.unrealengine.com/download
2. In the launcher, go to **Unreal Engine → Library → ＋** and install version **5.4+**
3. During installation, ensure these workloads are selected:
   - Core Engine
   - Starter Content
   - Templates

#### 3. Install Visual Studio 2022
1. Download from https://visualstudio.microsoft.com/downloads/
2. During installation, select these workloads:
   - **Desktop development with C++**
   - **Game development with C++**
   - **.NET desktop development** (for UE5 tools)
3. In "Individual components," also check:
   - Windows 10/11 SDK (latest)
   - MSVC v143 build tools

#### 4. Configure Git LFS
```bash
git lfs install
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.png"
git lfs track "*.jpg"
git lfs track "*.wav"
git lfs track "*.mp3"
git lfs track "*.ogg"
git lfs track "*.fbx"
git lfs track "*.obj"
```

#### 5. Generate Project Files
```bash
# Right-click the .uproject file → "Generate Visual Studio project files"
# OR from command line:
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="BiomeSurvivor.uproject" -game -engine
```

#### 6. Open in Visual Studio
- Open `BiomeSurvivor.sln`
- Set configuration to **Development Editor**
- Set platform to **Win64**
- Build solution (Ctrl+Shift+B)
- Press F5 to launch the editor

### Building the Project

#### Development Build (for testing)
```
Build Configuration: Development Editor
Platform: Win64
```

#### Shipping Build (for release)
```
Build Configuration: Shipping
Platform: Win64
Package via: File → Package Project → Windows (64-bit)
```

#### Dedicated Server Build
```
Build Configuration: Development Server / Shipping Server
Platform: Win64 or Linux
Add "BiomeSurvivorServer" target to Build.cs
```

---

## Project Structure

```
survival/
│
├── plan.md              → Master development plan (all specs)
├── README.md            → This file
├── requirements.md      → Software requirements & download links
│
├── Source/              → C++ source code
│   ├── Core/           → Core gameplay systems
│   │   ├── Player/     → Movement, stats, camera
│   │   ├── Inventory/  → Items, grid UI, equipment
│   │   ├── Crafting/   → Recipes, stations
│   │   ├── Building/   → Placement, structures, power
│   │   ├── Combat/     → Melee, ranged, stealth
│   │   ├── Survival/   → Temperature, nutrition, medical, sleep
│   │   └── Wildlife/   → Animal AI, species, spawning
│   ├── Networking/     → Singleplayer, LAN, co-op, replication
│   ├── World/          → Biomes, weather, terrain, resources
│   ├── UI/             → HUD, menus, inventory UI, settings
│   └── Audio/          → Music, SFX, ambient managers
│
├── Assets/             → Art assets (models, textures, audio, VFX)
├── Config/             → Game balance data (JSON/DataTables)
├── Docs/               → Extended documentation
├── Tests/              → Automated tests
├── Tools/              → Build scripts & utilities
└── Builds/             → Compiled game builds
```

> See [plan.md](plan.md) Section 4 for the complete expanded directory tree.

---

## Development Workflow

### Phase Overview (30 months total)
```
Phase 0  (Month 0-2)   → Pre-Production: project setup, free assets
Phase 1  (Month 2-4)   → Player: movement, stats, camera, HUD
Phase 2  (Month 4-6)   → Inventory: items, grid, gathering
Phase 3  (Month 6-8)   → Crafting: recipes, stations, durability
Phase 4  (Month 8-11)  → Building: placement, tiers, functional structures
Phase 5  (Month 11-15) → World: 7 biomes, weather, day/night, terrain
Phase 6  (Month 15-18) → Wildlife & Combat: AI, species, melee, ranged
Phase 7  (Month 18-20) → Survival: temperature, nutrition, medical
Phase 8  (Month 20-25) → Multiplayer: LAN, online co-op, servers
Phase 9  (Month 25-30) → Polish: optimization, audio, UI, beta, launch
```

### Branching Strategy
```
main           → stable, release-ready
develop        → integration branch
feature/XXX    → one branch per feature
bugfix/XXX     → bug fixes
release/X.X    → release candidates
```

### Commit Convention
```
feat: add inventory grid drag-and-drop
fix: resolve stamina not regenerating after dodge
art: import free deer model from Quaternius
docs: update crafting recipe table in plan.md
perf: optimize wildlife AI tick rate
test: add unit tests for temperature calculation
```

---

## Asset Strategy

> **Rule: Use FREE and open-source assets first. Replace with custom art only after gameplay is proven.**

### Recommended Free Asset Sources

| Source | URL | Type |
|--------|-----|------|
| UE Marketplace (Free) | https://unrealengine.com/marketplace/en-US/free | Models, materials, FX |
| Quixel Megascans | https://quixel.com/megascans/free | Photoscanned assets (free with UE) |
| Quaternius | https://quaternius.com/packs.html | CC0 low-poly animals & nature |
| Kenney.nl | https://kenney.nl/assets | CC0 game assets |
| Poly Haven | https://polyhaven.com/ | CC0 textures, HDRIs, models |
| ambientCG | https://ambientcg.com/ | CC0 PBR materials |
| Mixamo | https://mixamo.com/ | Free character animations |
| Freesound.org | https://freesound.org/ | CC0/CC-BY audio |
| Game-icons.net | https://game-icons.net/ | CC-BY game icons |
| Google Fonts | https://fonts.google.com/ | Open-source fonts |

> Full list with descriptions in [plan.md](plan.md) Section 2.1

---

## Documentation

| Document | Description |
|----------|-------------|
| [plan.md](plan.md) | **Master development plan** — all systems, specs, data structures, phases |
| [requirements.md](requirements.md) | Software requirements with download links |
| [Docs/Design/](Docs/Design/) | Game design documents (per-feature) |
| [Docs/Technical/](Docs/Technical/) | Architecture and networking docs |
| [Docs/Art/](Docs/Art/) | Art style guide and reference boards |

---

## Contributing

### For Team Members
1. Pick a task from the current phase in [plan.md](plan.md)
2. Create a feature branch: `git checkout -b feature/inventory-grid`
3. Implement the feature following UE5 conventions (see plan.md Appendix)
4. Write tests if applicable
5. Submit a pull request to `develop`
6. Request review from at least 1 team member

### Code Standards
- **C++ Naming:** PascalCase classes (prefix A/U/F), camelCase variables, UPPER_SNAKE constants
- **One class per file**
- **Use UE5 patterns:** Components, DataTables, Gameplay Ability System
- **No hardcoded values** — use Config/ DataTables or constants
- **Comment public functions** with `/** */` documentation comments

---

## License

This project is proprietary. All rights reserved.

Free third-party assets are used under their respective licenses (CC0, CC-BY). See individual asset directories for license files.

---

**Project Status:** 🔴 Pre-Production  
**Current Phase:** Phase 0 — Project Setup  
**Last Updated:** 2026-03-05
