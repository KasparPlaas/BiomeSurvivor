# BIOME SURVIVOR — Master Development Plan
<!-- AI-INSTRUCTION: This document is the single source of truth for the entire project.
     Each section is self-contained with explicit specifications so that any AI coding
     assistant (Copilot, Cursor, Claude, etc.) can generate implementation code directly
     from the descriptions below. All values, enums, ranges, and data structures are
     written in a machine-parseable style on purpose. -->

---

## 0. Document Metadata

| Key | Value |
|-----|-------|
| **Project Name** | Biome Survivor |
| **Version** | 2.0 |
| **Last Updated** | 2026-03-05 |
| **Status** | Pre-Production |
| **Engine** | Unreal Engine 5.4+ |
| **Language** | C++ (primary) · Blueprints (prototyping only) |
| **Target Platforms** | Windows 10/11 64-bit · Linux (dedicated server) |
| **Multiplayer** | Singleplayer · LAN (2-8) · Online Co-op (2-32) |
| **Perspective** | Third-person (default) / First-person (toggle) |
| **Art Direction** | Stylised realism — use **free & open-source assets** first, replace with custom art later |
| **License Model** | Premium ($24.99–$34.99 USD) |
| **Target Release** | Q3 2028 |
| **Estimated Dev Time** | ~30 months |

---

## 1. Project Overview

### 1.1 Elevator Pitch
> A 3D open-world survival game set across **7 distinct biomes** — from frozen tundra to scorching desert — where players gather resources, craft tools, build shelters, tame wildlife, and fight to stay alive. Playable solo **or** cooperatively over LAN and online.

### 1.2 Core Pillars (in priority order)
1. **Survival** — hunger, thirst, temperature, health, sleep
2. **Exploration** — massive open world with 7 hand-blended biomes
3. **Crafting & Building** — deep recipe tree + freeform base construction
4. **Combat** — skill-based melee/ranged fighting against wildlife
5. **Cooperation** — seamless singleplayer ↔ multiplayer transition

### 1.3 Reference Games
| Game | What to learn |
|------|---------------|
| The Long Dark | Temperature system, atmosphere, survival tension |
| Rust | Building system, multiplayer, base raiding |
| Valheim | Biome progression, co-op loop, stamina combat |
| Subnautica | Exploration reward, crafting progression |
| Don't Starve | Resource balance, sanity/comfort mechanics |
| Green Hell | Medical system, jungle survival |

---

## 2. Asset Strategy — FREE & Open Source First

> **IMPORTANT RULE:** Every phase starts with free/open-source placeholder assets.
> Custom assets replace placeholders only after gameplay is proven.

### 2.1 Free Asset Sources

| Category | Source | URL | Notes |
|----------|--------|-----|-------|
| **3D Models** | Unreal Marketplace (Free) | https://www.unrealengine.com/marketplace/en-US/free | Monthly free assets, "Permanently Free" collection |
| **3D Models** | Sketchfab (CC0/CC-BY) | https://sketchfab.com/search?type=models&licenses=7c23a1ba438d4306920229c12afcb5f9 | Filter by Creative Commons |
| **3D Models** | Kenney.nl | https://kenney.nl/assets | CC0 game assets (low-poly) |
| **3D Models** | Quaternius | https://quaternius.com/packs.html | CC0 low-poly animals, nature, characters |
| **3D Models** | Poly Haven | https://polyhaven.com/models | CC0 realistic models |
| **Textures** | Poly Haven Textures | https://polyhaven.com/textures | CC0 PBR textures |
| **Textures** | ambientCG | https://ambientcg.com/ | CC0 PBR materials |
| **Textures** | Texture Ninja | https://texture.ninja/ | Free high-res photos |
| **HDRIs** | Poly Haven HDRIs | https://polyhaven.com/hdris | CC0 environment maps |
| **Audio SFX** | Freesound.org | https://freesound.org/ | CC0/CC-BY audio clips |
| **Audio SFX** | Sonniss GDC Bundle | https://sonniss.com/gameaudiogdc | Free annual GDC bundles |
| **Music** | Incompetech | https://incompetech.com/music/ | CC-BY royalty-free music |
| **Music** | OpenGameArt | https://opengameart.org/ | CC0/CC-BY game music |
| **Fonts** | Google Fonts | https://fonts.google.com/ | Open-source fonts |
| **UI Icons** | Game-icons.net | https://game-icons.net/ | CC-BY 3.0 game icons |
| **Animations** | Mixamo | https://www.mixamo.com/ | Free character animations (Adobe account) |
| **Vegetation** | UE Megascans (free) | https://quixel.com/megascans/free | Free with Unreal Engine |

### 2.2 Asset Workflow
```
Phase 1-6:  Use FREE placeholder assets → focus on gameplay code
Phase 7-8:  Identify which placeholders need custom replacement
Phase 9:    Commission / create final art for key assets
Launch:     All player-facing assets are final quality
```

---

## 3. Development Environment & Tools

### 3.1 Primary OS — Windows 11 Professional
```
Reason: Best UE5 support, Visual Studio integration, DirectX 12, driver compatibility.
```

### 3.2 Required Software (see requirements.md for download links)

| Tool | Purpose | Cost |
|------|---------|------|
| Unreal Engine 5.4+ | Game engine | Free (5% royalty after $1M revenue) |
| Visual Studio 2022 Community | C++ IDE | Free |
| Git + Git LFS | Version control | Free |
| GitHub / GitLab | Remote repository | Free tier |
| Blender 4.0+ | 3D modeling & animation | Free |
| GIMP 2.10+ | 2D texture editing | Free |
| Audacity | Audio editing | Free |
| FMOD Studio | Audio middleware | Free (indie license) |
| Trello / Notion | Project management | Free tier |

### 3.3 Hardware — Development Machine (Recommended)
```
CPU:      Intel Core i9-12900K  /  AMD Ryzen 9 5950X
RAM:      64 GB DDR5
GPU:      NVIDIA RTX 4070 Ti (16 GB VRAM)
Storage:  1 TB NVMe SSD (project) + 2 TB SSD (assets)
Network:  Gigabit Ethernet
Monitor:  2560×1440 minimum
```

### 3.4 Hardware — Minimum Target PC (Player)
```
CPU:      Intel Core i5-8400  /  AMD Ryzen 5 2600
RAM:      12 GB
GPU:      NVIDIA GTX 1060 6 GB  /  AMD RX 580 8 GB
Storage:  40 GB SSD
DirectX:  12
Network:  Broadband (multiplayer only)
```

### 3.5 Hardware — Recommended Target PC (Player)
```
CPU:      Intel Core i7-10700K  /  AMD Ryzen 7 3700X
RAM:      16 GB
GPU:      NVIDIA RTX 3060 Ti  /  AMD RX 6700 XT
Storage:  40 GB NVMe SSD
DirectX:  12
Network:  Broadband (multiplayer only)
```

---

## 4. Project File Structure

```
survival/
├── plan.md                         ← THIS FILE — master development plan
├── README.md                       ← Project overview & quick-start guide
├── requirements.md                 ← All software, links, install instructions
│
├── Source/                          ← All game source code (C++ / Blueprints)
│   ├── Core/
│   │   ├── Player/
│   │   │   ├── Movement/           ← Walk, run, sprint, climb, swim, slide
│   │   │   ├── Stats/              ← HP, hunger, thirst, temp, stamina, morale
│   │   │   └── Camera/             ← FPS/TPS toggle, free-look, ADS
│   │   ├── Inventory/
│   │   │   ├── Items/              ← Item base class, item database, stacking
│   │   │   └── UI/                 ← Grid UI, drag-drop, tooltips, hotbar
│   │   ├── Crafting/
│   │   │   ├── Recipes/            ← JSON/DataTable recipe definitions
│   │   │   └── Stations/           ← Workbench, forge, campfire, alchemy
│   │   ├── Building/
│   │   │   ├── Structures/         ← Wall, floor, roof, door, window pieces
│   │   │   ├── Placement/          ← Snap system, ghost preview, validation
│   │   │   └── Power/              ← Generator, solar, wiring, batteries
│   │   ├── Combat/
│   │   │   ├── Melee/              ← Combo system, block, parry, dodge
│   │   │   ├── Ranged/             ← Bow, throwables, firearms, projectiles
│   │   │   └── Stealth/            ← Detection, visibility, stealth kills
│   │   ├── Survival/
│   │   │   ├── Temperature/        ← Biome temp, clothing insulation, fire
│   │   │   ├── Nutrition/          ← Food types, balanced diet, spoilage
│   │   │   ├── Medical/            ← Injuries, treatment, infection
│   │   │   └── Sleep/              ← Rest quality, beds, comfort
│   │   └── Wildlife/
│   │       ├── AI/                 ← Behavior trees, pack AI, flee/attack
│   │       ├── Animals/            ← Per-species stats, models, drops
│   │       └── Spawning/           ← Biome-based spawn tables, density
│   ├── Networking/
│   │   ├── Singleplayer/           ← Save/load, offline mode
│   │   ├── LAN/                    ← Host discovery, local sessions
│   │   ├── Coop/                   ← Dedicated server, P2P, matchmaking
│   │   └── Replication/            ← Actor replication, RPCs, ownership
│   ├── World/
│   │   ├── Biomes/
│   │   │   ├── TemperateWoods/     ← Starting biome — low threat
│   │   │   ├── ArcticTundra/       ← Extreme cold biome
│   │   │   ├── Desert/             ← Extreme heat biome
│   │   │   ├── Jungle/             ← Dense tropical biome
│   │   │   ├── Mountain/           ← High altitude biome
│   │   │   ├── Swamp/              ← Wetlands biome
│   │   │   └── Savanna/            ← Open grassland biome
│   │   ├── Weather/                ← Rain, snow, sandstorm, fog, tornado
│   │   ├── DayNightCycle/          ← Sun position, sky color, lighting
│   │   ├── Terrain/                ← Heightmaps, landscape layers, foliage
│   │   └── Resources/              ← Spawn rules, respawn timers, density
│   ├── UI/
│   │   ├── HUD/                    ← Health bars, compass, status icons
│   │   ├── Menus/                  ← Main menu, pause, settings, server browser
│   │   ├── Inventory/              ← Player inventory screen
│   │   ├── Crafting/               ← Crafting menu, recipe browser
│   │   ├── Building/               ← Building wheel, piece selector
│   │   ├── Map/                    ← Fog-of-war map, markers, compass
│   │   └── Settings/               ← Graphics, audio, controls, accessibility
│   └── Audio/
│       ├── Music/                  ← Adaptive music manager
│       ├── SFX/                    ← Gameplay sound effects
│       └── Ambient/                ← Per-biome ambient soundscapes
│
├── Assets/                          ← All art assets (models, textures, audio)
│   ├── Models/
│   │   ├── Characters/             ← Player model, NPC models
│   │   ├── Animals/                ← All animal species
│   │   ├── Props/                  ← Interactable world objects
│   │   ├── Buildings/              ← Building pieces (wall, floor, etc.)
│   │   ├── Weapons/                ← Axe, bow, spear, sword models
│   │   ├── Tools/                  ← Pickaxe, hammer, knife models
│   │   ├── Vegetation/             ← Trees, bushes, grass, flowers
│   │   └── Terrain/                ← Rock formations, cliffs
│   ├── Textures/
│   │   ├── Characters/
│   │   ├── Environment/
│   │   ├── UI/
│   │   └── Items/
│   ├── Materials/
│   │   ├── Terrain/                ← Landscape material layers
│   │   ├── Water/                  ← Ocean, river, lake shaders
│   │   └── Weather/                ← Rain, snow, fog materials
│   ├── Animations/
│   │   ├── Player/                 ← Locomotion, combat, interact
│   │   ├── Animals/                ← Walk, run, attack, death, idle
│   │   └── Combat/                 ← Swing, block, dodge, ranged
│   ├── Audio/
│   │   ├── Music/                  ← Background tracks per mood
│   │   ├── SFX/                    ← Footsteps, hits, crafting, UI
│   │   └── Ambient/                ← Wind, rain, birds, insects
│   ├── VFX/
│   │   ├── Fire/                   ← Campfire, torch, burning
│   │   ├── Weather/                ← Rain, snow, sand particles
│   │   └── Combat/                 ← Blood, impact, sparks
│   └── UI/
│       ├── Icons/                  ← Item icons, status effect icons
│       ├── Fonts/                  ← UI typefaces
│       └── Widgets/                ← Reusable UI components
│
├── Config/                          ← Data-driven configuration files
│   ├── GameSettings/               ← Default settings, difficulty presets
│   ├── Balancing/                  ← Damage tables, stat curves, XP rates
│   ├── Items/                      ← Item database (JSON/DataTables)
│   ├── Recipes/                    ← Crafting recipe definitions
│   ├── Biomes/                     ← Per-biome config (temp, resources, spawns)
│   └── Animals/                    ← Animal stat sheets, loot tables
│
├── Docs/                            ← Extended documentation
│   ├── Design/                     ← GDD sections, feature specs
│   ├── Technical/                  ← Architecture docs, networking diagrams
│   ├── Art/                        ← Style guides, reference boards
│   └── API/                        ← Code API documentation
│
├── Tests/                           ← Automated testing
│   ├── Unit/                       ← Per-class unit tests
│   ├── Integration/                ← System interaction tests
│   └── Performance/                ← FPS benchmarks, memory profiling
│
├── Tools/                           ← Dev utilities & scripts
│   ├── Scripts/                    ← Build scripts, data converters
│   └── Importers/                  ← Asset import pipelines
│
└── Builds/                          ← Compiled output
    ├── Windows/
    ├── Linux/
    └── Server/                     ← Dedicated server builds
```

---

## 5. Core Systems — Detailed Specifications

<!-- AI-INSTRUCTION: Each system below includes data structures, enums, and numeric
     values that can be directly translated into C++ classes and UE5 Blueprints. -->

### 5.1 PLAYER CHARACTER

#### 5.1.1 Movement System
**Location:** `Source/Core/Player/Movement/`

```
MOVEMENT_MODES:
  - Walk:       speed = 250 cm/s,   stamina_cost = 0/s
  - Run:        speed = 450 cm/s,   stamina_cost = 5/s
  - Sprint:     speed = 650 cm/s,   stamina_cost = 15/s
  - Crouch:     speed = 150 cm/s,   stamina_cost = 0/s
  - Prone:      speed = 75 cm/s,    stamina_cost = 0/s
  - Swim:       speed = 200 cm/s,   stamina_cost = 8/s
  - Climb:      speed = 100 cm/s,   stamina_cost = 12/s

ACTIONS:
  - Jump:       force = 500,        stamina_cost = 10 (flat)
  - Dodge Roll: distance = 300 cm,  stamina_cost = 20 (flat), i-frames = 0.3s
  - Slide:      triggered on slope > 35°, speed = acceleration from gravity

MODIFIERS:
  - Encumbrance:  if carry_weight > 80% of max_capacity → speed × 0.7
  - Injury:       fracture → speed × 0.5, no sprint
  - Wet:          swim exit → speed × 0.85 for 60s
  - Temperature:  hypothermia_stage_2 → speed × 0.6

CAMERA:
  - Default: Third-person, over-right-shoulder, distance = 250 cm
  - Toggle: First-person (V key default)
  - ADS: Zoom to first-person when aiming ranged weapon
  - Free-look: Hold Alt to look around without changing move direction
```

#### 5.1.2 Player Stats
**Location:** `Source/Core/Player/Stats/`

```
STAT: Health
  - max = 100, regen = 0.5/s (when well-fed and rested, no bleeding)
  - death at 0

STAT: Hunger
  - max = 100, drain = 1.0/min (base), sprint multiplier × 1.5
  - at 0: health drains 2.0/min

STAT: Thirst
  - max = 100, drain = 1.5/min (base), hot biome multiplier × 2.0
  - at 0: health drains 3.0/min

STAT: Stamina
  - max = 100, regen = 8/s (when not sprinting), pause after depletion = 2s
  - affected by hunger (< 25 hunger → regen × 0.5)

STAT: Temperature (body)
  - comfortable range = 35.0°C – 37.5°C
  - below 35°C → Cold stages (1-3), above 39°C → Heat stages (1-3)
  - affected by: biome_ambient_temp + clothing_insulation + fire_proximity + wetness + wind

STAT: Comfort / Morale
  - max = 100, affects stamina regen bonus (+20% at comfort > 80)
  - increased by: shelter, fire, cooked food, bed rest
  - decreased by: injuries, darkness, isolation, bad food

STAT: Weight Capacity
  - base = 40.0 kg
  - backpack adds 10-30 kg depending on tier
  - over-encumbered at max → cannot sprint, speed × 0.5
```

#### 5.1.3 Status Effects
```
EFFECT: Hypothermia
  - Stage 1 (body_temp 33-35°C): blurred vision edges, stamina regen × 0.7
  - Stage 2 (body_temp 30-33°C): screen shake, speed × 0.6, stamina regen × 0.4
  - Stage 3 (body_temp < 30°C): health drain 5/min, blackout risk

EFFECT: Hyperthermia
  - Stage 1 (39-40°C): increased thirst drain × 2, stamina regen × 0.7
  - Stage 2 (40-41°C): blurred vision, thirst drain × 3
  - Stage 3 (> 41°C): health drain 5/min, hallucination effects

EFFECT: Bleeding
  - health drain = 3/min per wound, max 3 stacks
  - cured by: Bandage item (stops 1 stack)

EFFECT: Fracture
  - speed × 0.5, cannot sprint or jump
  - cured by: Splint item + 10 min game-time recovery

EFFECT: Poisoning
  - health drain = 2/min for 5 min, nausea screen effect
  - cured by: Antidote item

EFFECT: Infection
  - starts 30 min after untreated wound, health drain = 1/min (escalating)
  - cured by: Antibiotics item

EFFECT: Starvation
  - triggered at hunger = 0, health drain 2/min, no stamina regen

EFFECT: Dehydration
  - triggered at thirst = 0, health drain 3/min, blurred vision
```

---

### 5.2 INVENTORY SYSTEM
**Location:** `Source/Core/Inventory/`

#### 5.2.1 Data Structure
```
CLASS: InventoryComponent
  - grid_size: 10 columns × 6 rows (expandable by backpack)
  - max_weight: float (from player stat)
  - slots: Array<InventorySlot>

CLASS: InventorySlot
  - item_id: FName (references ItemDatabase)
  - quantity: int (1 for non-stackable, up to stack_max)
  - durability: float (0.0 – 1.0, where 1.0 = new)
  - quality: enum { Common, Uncommon, Rare, Epic }

CLASS: ItemDefinition (DataTable row)
  - item_id: FName (unique)
  - display_name: FText
  - description: FText
  - icon: Texture2D
  - mesh: StaticMesh
  - weight: float (kg)
  - stack_max: int (1 = non-stackable, 99 for resources)
  - category: enum { Resource, Tool, Weapon, Armor, Clothing, Consumable, Building, Utility }
  - rarity: enum { Common, Uncommon, Rare, Epic }
  - durability_max: float (0 = indestructible)
  - use_action: enum { None, Eat, Drink, Equip, Place, Heal, Fuel }
  - biome_source: Array<BiomeType> (which biomes this item spawns in)
```

#### 5.2.2 Equipment Slots
```
SLOT: Head          → helmets, hats, hoods           → armor + temp insulation
SLOT: Torso         → jackets, vests, chest armor     → armor + temp insulation
SLOT: Legs          → pants, leg armor                → armor + temp insulation
SLOT: Feet          → boots, sandals                  → armor + temp insulation + speed modifier
SLOT: Hands         → gloves                          → gathering speed bonus
SLOT: Back          → backpack                        → weight capacity bonus
SLOT: ToolBelt      → 4 quick-use slots               → Q/E cycle or 1-4 hotkeys
SLOT: PrimaryWeapon → melee or ranged weapon           → LMB attack
SLOT: SecondaryWeapon → sidearm or melee backup        → swap key
```

#### 5.2.3 UI Features
```
- Drag & drop items between slots
- Right-click context menu: Use, Equip, Drop, Split Stack, Inspect
- Shift+click to quick-transfer to/from containers
- Sort button: by name, weight, rarity, category
- Search bar: filter by item name
- Item tooltip: name, description, stats, durability bar, weight
- 3D item preview: rotate model in inspect mode
- Container interaction: side-by-side inventory view
- Hotbar: slots 1-8 at bottom of HUD, drag from inventory
```

---

### 5.3 RESOURCE SYSTEM
**Location:** `Source/World/Resources/`

#### 5.3.1 Resource Nodes
```
RESOURCE: Wood
  - source: Trees (hit with axe), fallen logs, branches on ground
  - yield_per_hit: hands=1, stone_axe=3, iron_axe=5, steel_axe=8
  - tree_hp: 100 (small), 200 (medium), 400 (large)
  - respawn_time: 30 min game-time

RESOURCE: Stone
  - source: Rock nodes, boulders, cliff faces
  - yield_per_hit: hands=0, stone_pickaxe=2, iron_pickaxe=4, steel_pickaxe=7
  - node_hp: 150
  - respawn_time: 45 min game-time

RESOURCE: Plant Fiber
  - source: Bushes, tall grass (interact key)
  - yield_per_gather: 3-5 (random)
  - respawn_time: 15 min game-time

RESOURCE: Iron Ore
  - source: Metal rock nodes (grey-orange), caves, mountain biome
  - yield_per_hit: iron_pickaxe=2, steel_pickaxe=4
  - node_hp: 200
  - respawn_time: 60 min game-time

RESOURCE: Copper Ore
  - source: Copper rock nodes (green-brown), caves
  - yield_per_hit: stone_pickaxe=1, iron_pickaxe=3
  - node_hp: 150
  - respawn_time: 60 min game-time

RESOURCE: Fresh Water
  - source: Rivers, lakes (collect with container), rain collector
  - must boil at campfire to make safe (removes disease risk)

RESOURCE: Berries
  - source: Berry bushes (interact key), various biomes
  - yield: 2-4, respawn: 20 min
  - nutrition: hunger +5, thirst +2

RESOURCE: Meat (Raw)
  - source: Animal carcass (harvest with knife)
  - yield: varies by animal size (rabbit=2, deer=6, bear=12)
  - spoilage: 15 min game-time unless dried/cooked/refrigerated
```

---

### 5.4 CRAFTING SYSTEM
**Location:** `Source/Core/Crafting/`

#### 5.4.1 Crafting Station Types
```
STATION: Personal Crafting (no station needed)
  - recipes: basic tools, bandages, torch, campfire, plant fiber rope

STATION: Campfire
  - recipes: cooked meat, boiled water, charcoal, simple stews
  - fuel: wood (1 wood = 5 min burn), coal (1 coal = 15 min burn)
  - build_cost: 10 stone, 5 wood

STATION: Workbench
  - recipes: advanced tools, basic weapons, building materials, furniture
  - build_cost: 20 wood, 10 stone, 5 plant_fiber

STATION: Forge
  - recipes: metal tools, metal weapons, metal building pieces, nails
  - fuel: coal or charcoal
  - build_cost: 30 stone, 10 iron_ore, 5 wood

STATION: Tanning Rack
  - recipes: leather, cured hide, leather armor, leather bags
  - build_cost: 15 wood, 10 plant_fiber

STATION: Alchemy Station
  - recipes: medicine, antidotes, antibiotics, healing salves, poisons
  - build_cost: 20 wood, 10 stone, 5 glass (crafted from sand at forge)

STATION: Sewing Station
  - recipes: cloth clothing, insulated clothing, bandages, backpacks
  - build_cost: 15 wood, 10 plant_fiber, 5 leather
```

#### 5.4.2 Example Recipes (JSON-style)
```json
[
  {
    "recipe_id": "stone_axe",
    "display_name": "Stone Axe",
    "station": "Personal",
    "ingredients": [
      { "item": "stone", "quantity": 3 },
      { "item": "wood", "quantity": 2 },
      { "item": "plant_fiber", "quantity": 5 }
    ],
    "output": { "item": "stone_axe", "quantity": 1 },
    "craft_time_seconds": 5,
    "unlock_method": "default"
  },
  {
    "recipe_id": "iron_pickaxe",
    "display_name": "Iron Pickaxe",
    "station": "Workbench",
    "ingredients": [
      { "item": "iron_ingot", "quantity": 4 },
      { "item": "wood", "quantity": 3 },
      { "item": "leather", "quantity": 2 }
    ],
    "output": { "item": "iron_pickaxe", "quantity": 1 },
    "craft_time_seconds": 12,
    "unlock_method": "discover_iron"
  },
  {
    "recipe_id": "cooked_meat",
    "display_name": "Cooked Meat",
    "station": "Campfire",
    "ingredients": [
      { "item": "raw_meat", "quantity": 1 }
    ],
    "output": { "item": "cooked_meat", "quantity": 1 },
    "craft_time_seconds": 8,
    "unlock_method": "default"
  },
  {
    "recipe_id": "iron_ingot",
    "display_name": "Iron Ingot",
    "station": "Forge",
    "ingredients": [
      { "item": "iron_ore", "quantity": 2 },
      { "item": "charcoal", "quantity": 1 }
    ],
    "output": { "item": "iron_ingot", "quantity": 1 },
    "craft_time_seconds": 15,
    "unlock_method": "build_forge"
  },
  {
    "recipe_id": "bandage",
    "display_name": "Bandage",
    "station": "Personal",
    "ingredients": [
      { "item": "cloth", "quantity": 2 }
    ],
    "output": { "item": "bandage", "quantity": 1 },
    "craft_time_seconds": 3,
    "unlock_method": "default"
  }
]
```

#### 5.4.3 Item Durability & Repair
```
- All tools/weapons/armor have durability (0.0 to 1.0)
- Each use reduces durability by item-specific amount (e.g., axe_hit = -0.005)
- At durability 0.0 → item breaks, cannot be used, can still be repaired
- Repair cost = 50% of original recipe materials
- Repair at: appropriate crafting station
- Sharpening stone: restores 0.2 durability to bladed weapons (personal use)
```

---

### 5.5 BUILDING SYSTEM
**Location:** `Source/Core/Building/`

#### 5.5.1 Placement Mechanics
```
MODE: Enter building mode via hotkey (B)
PREVIEW: Ghost mesh shows placement position (green = valid, red = invalid)
SNAP: Pieces snap to each other when within 50 cm tolerance
FREE_PLACE: Toggle with (G) key for free placement (furniture, decorations)

VALIDATION_RULES:
  - Foundation required under walls (unless supported by terrain)
  - Walls need foundation or floor support
  - Roofs need wall support
  - Max building height: 10 stories (30 meters)
  - Cannot build inside rock/terrain
  - Minimum distance from other players' bases: 20 meters (server configurable)

ROTATE: Mouse scroll or R key (15° increments, hold Shift for 5°)
DESTROY: Hold (X) + confirm to dismantle own structures (returns 50% materials)
```

#### 5.5.2 Building Pieces
```
TIER 1 — THATCH/WOOD:
  Material cost: Wood + Plant Fiber
  HP: 500
  Insulation: +5°C
  Build time: Instant placement

  Pieces: Foundation, Wall, Wall_Window, Wall_Door, Floor, Roof_Slant,
          Roof_Flat, Stairs, Ramp, Doorframe, Door, Ladder, Fence

TIER 2 — STONE:
  Material cost: Stone + Wood
  HP: 2000
  Insulation: +10°C
  Upgrade from: Tier 1 (pay material difference)

  Pieces: Same set as Tier 1 + Arch, Pillar, Balcony

TIER 3 — METAL/REINFORCED:
  Material cost: Iron Ingot + Stone
  HP: 5000
  Insulation: +8°C (metal conducts — less than stone)
  Upgrade from: Tier 2 (pay material difference)

  Pieces: Same set as Tier 2 + Reinforced_Door, Hatch, Metal_Shutter
```

#### 5.5.3 Functional Structures
```
STRUCTURE: Campfire          → cooking station + warmth (3m radius, +15°C)
STRUCTURE: Bed               → respawn point + full rest (8-hour sleep)
STRUCTURE: Sleeping Bag      → one-time respawn point + basic rest
STRUCTURE: Storage Chest     → 24 slot container, Tier: wood/stone/metal
STRUCTURE: Rain Collector    → passive water generation during rain
STRUCTURE: Drying Rack       → preserves raw meat (no spoilage, 5 min process)
STRUCTURE: Compost Bin       → converts plant waste → fertilizer (for farming)
STRUCTURE: Planter Box       → grow crops: wheat, carrots, berries, herbs
STRUCTURE: Animal Pen        → tame and breed passive animals (chickens, goats)
STRUCTURE: Spike Trap        → damages wildlife/players walking through (200 dmg)
STRUCTURE: Wall Torch        → light source, 5m radius, burns wood fuel
STRUCTURE: Lantern           → portable/placeable light, burns animal fat
```

#### 5.5.4 Power System (Late-Game)
```
GENERATOR:    Burns fuel → produces 100W, fuel = 1 unit per 10 min
SOLAR_PANEL:  Produces 50W during daytime only
WIND_TURBINE: Produces 30-80W based on weather (mountain biome bonus)
BATTERY:      Stores 500Wh, charges from generators/solar/wind
WIRE:         Connects power source → powered structure (max 50m length)
REFRIGERATOR: Requires 20W, prevents all food spoilage inside
ELECTRIC_LIGHT: Requires 5W, brighter than torches, no fuel needed
```

---

### 5.6 BIOME SPECIFICATIONS
**Location:** `Source/World/Biomes/`

```
BIOME CONFIG SCHEMA:
{
  "biome_id": string,
  "display_name": string,
  "ambient_temp_day": float (°C),
  "ambient_temp_night": float (°C),
  "temp_variation": float (random ± range),
  "humidity": float (0.0-1.0),
  "wind_base": float (m/s),
  "terrain_type": string,
  "vegetation_density": float (0.0-1.0),
  "water_sources": string[],
  "resources": ResourceSpawnRule[],
  "wildlife": AnimalSpawnRule[],
  "weather_events": WeatherEvent[],
  "hazards": string[],
  "threat_level": int (1-5),
  "music_theme": string,
  "ambient_sounds": string[]
}
```

#### BIOME 1: Temperate Woods (STARTING BIOME)
```
biome_id:          "temperate_woods"
ambient_temp_day:  18.0°C
ambient_temp_night: 8.0°C
humidity:          0.5
wind_base:         3.0 m/s
vegetation_density: 0.8
threat_level:      1

TERRAIN: Rolling hills, oak/pine forests, meadows, streams, small lakes
RESOURCES: Wood (abundant), Stone (common), Berries (common), Plant Fiber (abundant),
           Mushrooms (uncommon), Iron Ore (rare, in caves)
WILDLIFE:
  - Deer (passive, common) → raw meat ×4, hide ×2
  - Rabbit (passive, common) → raw meat ×1, hide ×1
  - Wolf (aggressive-pack, uncommon) → raw meat ×3, hide ×2, fangs ×2
  - Bear (territorial, rare) → raw meat ×8, hide ×4, claws ×2, fat ×3
  - Fox (passive, uncommon) → raw meat ×2, hide ×1
  - Birds (ambient, common) → feather drops on ground
WEATHER: Rain (common), Fog (uncommon), Thunderstorm (rare)
HAZARDS: Wolves at night, occasional bear encounter
```

#### BIOME 2: Arctic Tundra
```
biome_id:          "arctic_tundra"
ambient_temp_day:  -12.0°C
ambient_temp_night: -28.0°C
humidity:          0.2
wind_base:         8.0 m/s
vegetation_density: 0.1
threat_level:      4

TERRAIN: Flat snow plains, frozen lakes, ice caves, sparse conifers, glaciers
RESOURCES: Wood (scarce — sparse trees), Stone (common under snow), Ice (abundant),
           Arctic Fish (common in ice holes), Seal Blubber (from seals)
WILDLIFE:
  - Polar Bear (aggressive, uncommon) → raw meat ×10, thick hide ×4, fat ×5
  - Arctic Wolf (aggressive-pack, common) → raw meat ×3, fur ×3
  - Seal (passive, common) → raw meat ×4, blubber ×3, hide ×2
  - Arctic Hare (passive, common) → raw meat ×1, fur ×1
  - Snowy Owl (ambient, uncommon) → feathers on ground
WEATHER: Blizzard (common), Heavy Snow (common), Clear & Freezing (common)
HAZARDS: Hypothermia (constant threat), thin ice (fall into water), whiteout blizzards
SPECIAL: Clothing must provide ≥ 25°C insulation to survive daytime
```

#### BIOME 3: Desert
```
biome_id:          "desert"
ambient_temp_day:  45.0°C
ambient_temp_night: 8.0°C
humidity:          0.05
wind_base:         5.0 m/s
vegetation_density: 0.05
threat_level:      4

TERRAIN: Sand dunes, rocky outcrops, dry riverbeds, rare oases, canyons
RESOURCES: Stone (common), Sand (abundant), Cactus (uncommon — gives water + fiber),
           Sulfur (rare, canyon caves), Rare Minerals (caves)
WILDLIFE:
  - Rattlesnake (aggressive, common) → venom sac ×1, raw meat ×1, scales ×2
  - Scorpion (aggressive, common) → venom sac ×1, chitin ×2
  - Coyote (neutral, uncommon) → raw meat ×3, hide ×2
  - Vulture (passive, ambient) → feathers on ground
  - Camel (passive, rare) → tameable mount, raw meat ×6, hide ×3
WEATHER: Sandstorm (common), Heat Wave (common), Clear Night (cold)
HAZARDS: Dehydration (×2 thirst drain), heatstroke, sandstorms (reduce visibility + damage)
SPECIAL: Water is critical — canteen and shade management are core gameplay
```

#### BIOME 4: Jungle / Rainforest
```
biome_id:          "jungle"
ambient_temp_day:  32.0°C
ambient_temp_night: 24.0°C
humidity:          0.95
wind_base:         1.0 m/s
vegetation_density: 0.95
threat_level:      3

TERRAIN: Dense canopy, vines, rivers, waterfalls, ancient ruins, muddy trails
RESOURCES: Hardwood (abundant), Exotic Fruits (common), Rare Herbs (uncommon),
           Rubber (rare — rubber trees), Vines (common — rope substitute)
WILDLIFE:
  - Jaguar (aggressive-stealth, uncommon) → raw meat ×5, pelt ×3, claws ×2
  - Monkey (passive, common) → alerts nearby animals (alarm system)
  - Venomous Snake (aggressive, common) → venom sac ×1, raw meat ×1
  - Giant Spider (aggressive, uncommon) → venom ×2, silk ×3
  - Parrot (ambient, common) → feathers on ground
  - Frog (passive, common) → poison secretion ×1 (alchemy ingredient)
WEATHER: Heavy Rain (constant), Thunderstorm (common), Fog (morning)
HAZARDS: Disease from insects, poisonous plants, slippery terrain, poor visibility
SPECIAL: Wet status almost permanent — need waterproof clothing
```

#### BIOME 5: Mountain / Alpine
```
biome_id:          "mountain"
ambient_temp_day:  5.0°C
ambient_temp_night: -8.0°C
humidity:          0.3
wind_base:         10.0 m/s
vegetation_density: 0.3
threat_level:      3

TERRAIN: Steep slopes, cliffs, caves (mineral-rich), alpine meadows, snow peaks
RESOURCES: Iron Ore (abundant), Copper Ore (common), Rare Minerals (uncommon),
           Stone (abundant), Mountain Herbs (rare — medicinal)
WILDLIFE:
  - Mountain Lion (aggressive, uncommon) → raw meat ×5, pelt ×3
  - Mountain Goat (passive, common) → raw meat ×3, hide ×2, horn ×2
  - Eagle (aggressive if near nest, rare) → feathers ×4, talons ×2
  - Marmot (passive, common) → raw meat ×1, fur ×1
WEATHER: High Winds (constant), Snow (common), Avalanche (rare event)
HAZARDS: Fall damage (steep terrain), altitude sickness (above 2000m), avalanches
SPECIAL: Best biome for metal ore — richest mining. Climbing gear recommended.
```

#### BIOME 6: Swamp / Wetlands
```
biome_id:          "swamp"
ambient_temp_day:  22.0°C
ambient_temp_night: 16.0°C
humidity:          0.9
wind_base:         2.0 m/s
vegetation_density: 0.7
threat_level:      3

TERRAIN: Shallow murky water, mangroves, cypress trees, mud islands, fog
RESOURCES: Reeds (abundant), Fish (common), Algae (common), Peat (uncommon — fuel),
           Alligator Hide (from gators), Medicinal Moss (rare)
WILDLIFE:
  - Alligator (aggressive-territorial, common) → raw meat ×6, tough hide ×4
  - Water Snake (aggressive, common) → venom sac ×1, raw meat ×1
  - Frog (passive, common) → poison secretion ×1
  - Mosquito Swarm (ambient hazard) → disease risk per exposure
  - Heron (ambient, common) → feathers on ground
  - Leech (ambient hazard) → attached = health drain 1/min until removed
WEATHER: Fog (constant morning), Rain (common), Muggy Heat (afternoon)
HAZARDS: Disease (high), leeches, difficult movement (mud = speed × 0.6), hidden water depth
SPECIAL: Unique alchemy ingredients. Movement is the main challenge.
```

#### BIOME 7: Savanna / Grassland
```
biome_id:          "savanna"
ambient_temp_day:  30.0°C
ambient_temp_night: 18.0°C
humidity:          0.3
wind_base:         6.0 m/s
vegetation_density: 0.4
threat_level:      3

TERRAIN: Vast open grass plains, acacia trees, watering holes, termite mounds, rocky kopjes
RESOURCES: Tall Grass (abundant — plant fiber), Acacia Wood (common),
           Animal Hides (from hunts), Dry Grass (fuel)
WILDLIFE:
  - Lion (aggressive-pack, uncommon) → raw meat ×6, pelt ×3, fangs ×2
  - Elephant (neutral-territorial, rare) → tusks ×2, thick hide ×6, raw meat ×15
  - Buffalo/Bison (neutral-herding, common) → raw meat ×8, hide ×4, horn ×2
  - Hyena (aggressive-pack, common) → raw meat ×3, hide ×2
  - Zebra (passive-herding, common) → raw meat ×5, hide ×3
  - Giraffe (passive, rare) → hide ×4, raw meat ×8 (difficult to hunt — tall)
WEATHER: Wildfire (rare — spreads through grass), Dry Heat (common), Rare Rain
HAZARDS: Predator packs (lions, hyenas), wildfires, open terrain (no hiding)
SPECIAL: Best biome for animal resources. Hunting and taming gameplay focus.
```

#### 5.6.1 Biome Transitions
```
- Transition zones = 200m wide gradient between biomes
- Temperature blends linearly across transition
- Vegetation mixes both biome types
- Resource nodes from both biomes appear at reduced density
- Wildlife from both biomes can wander into transition
- Weather blends (e.g., rain from jungle side, clear from savanna side)
```

---

### 5.7 WILDLIFE & COMBAT
**Location:** `Source/Core/Combat/` and `Source/Core/Wildlife/`

#### 5.7.1 Animal AI — Behavior Tree Structure
```
BEHAVIOR: Passive (deer, rabbit, zebra)
  Priority 1: Flee from threats within detection_radius
  Priority 2: Wander to food sources
  Priority 3: Move to water every 30 min game-time
  Priority 4: Idle animation / grazing
  detection_radius: 30m (sight), 15m (sound)

BEHAVIOR: Neutral (elephant, buffalo)
  Priority 1: If attacked → fight back for 30s, then flee if HP < 40%
  Priority 2: Wander in herd (stay within 20m of herd center)
  Priority 3: Graze / drink
  detection_radius: 20m (sight), 10m (sound)

BEHAVIOR: Aggressive (wolves, lions, snakes)
  Priority 1: Hunt prey (including player) if hunger < 50%
  Priority 2: Rest at den/lair if hunger > 80%
  Priority 3: Patrol territory
  detection_radius: 40m (sight), 25m (sound)
  attack_range: melee-specific per animal

BEHAVIOR: Territorial (bear)
  Priority 1: If player within 15m of den → roar warning
  Priority 2: If player still within 10m after 5s → charge attack
  Priority 3: Chase up to 50m then return to territory
  Priority 4: Normal wander within territory

BEHAVIOR: Pack (wolves, hyenas, lions)
  - Alpha leads, pack follows within 15m
  - Alpha initiates attack → pack joins
  - If alpha dies → pack flees
  - Pack size: 3-6 members
  - Flanking behavior: pack members circle the target
```

#### 5.7.2 Melee Combat
```
ATTACK: Light Attack (LMB)
  - damage = weapon_base_damage × 1.0
  - speed = fast (0.4s)
  - stamina_cost = 8
  - can chain up to 3-hit combo (damage: 1.0x, 1.0x, 1.5x)

ATTACK: Heavy Attack (Hold LMB)
  - damage = weapon_base_damage × 2.0
  - speed = slow (1.0s charge + 0.5s swing)
  - stamina_cost = 20
  - staggers target

DEFEND: Block (RMB hold)
  - reduces incoming damage by 60% (shield) or 30% (weapon)
  - stamina cost = incoming_damage × 0.5
  - if stamina depleted while blocking → guard broken (stunned 1.5s)

DEFEND: Parry (RMB tap within 0.2s of incoming hit)
  - negates all damage
  - staggers attacker for 1.0s
  - stamina_cost = 10

MOVEMENT: Dodge Roll (Space + direction while in combat)
  - i-frames = 0.3s
  - stamina_cost = 20
  - distance = 3m
  - cooldown = 0.5s

WEAPON STATS EXAMPLE:
  Stone Axe:      damage=15, speed=0.8, reach=1.5m, durability_per_hit=0.008
  Iron Sword:     damage=30, speed=0.6, reach=2.0m, durability_per_hit=0.005
  Wooden Spear:   damage=20, speed=0.7, reach=3.0m, durability_per_hit=0.010
  Iron Club:      damage=35, speed=1.0, reach=1.5m, durability_per_hit=0.006
```

#### 5.7.3 Ranged Combat
```
WEAPON: Bow
  - damage: depends on arrow type (stone=20, iron=35, fire=25+burn)
  - draw_time: 1.2s (full draw for max damage, quick shot = 50% damage)
  - arrow_speed: 3000 cm/s
  - arrow_gravity: -980 cm/s² (realistic arc)
  - stamina_cost: 12 per shot
  - max_effective_range: 60m

WEAPON: Throwable (Spear, Knife, Rock)
  - damage: spear=40, knife=25, rock=10
  - throw_speed: 2000 cm/s
  - gravity: -980 cm/s²
  - retrievable: spear=yes, knife=yes, rock=no
  - stamina_cost: 15

WEAPON: Firearms (Late-game, scarce ammo)
  - Pistol:  damage=50, rate_of_fire=2/s, magazine=8, range=40m
  - Rifle:   damage=80, rate_of_fire=0.8/s, magazine=5, range=100m
  - Shotgun: damage=90 (spread), rate_of_fire=0.6/s, magazine=2, range=15m
  - Ammo must be crafted (gunpowder from sulfur + charcoal, casings from metal)
```

#### 5.7.4 Stealth System
```
DETECTION MODEL:
  - Each animal has sight_radius and sound_radius
  - Player visibility = base_visibility × posture_modifier × lighting_modifier
    - Standing: ×1.0, Crouching: ×0.5, Prone: ×0.2
    - Daylight: ×1.0, Dusk/Dawn: ×0.6, Night: ×0.3
    - Behind foliage: ×0.5
  - Player noise = movement_noise + equipment_noise
    - Sprint: loud (30m), Run: medium (15m), Walk: quiet (5m), Crouch-walk: minimal (2m)
    - Metal armor: +10m to all noise levels, Cloth: +0m addition

STEALTH KILL:
  - Approach undetected from behind within 2m → prompt appears
  - Instant kill on small/medium animals
  - ×3 damage on large animals (not instant kill)
  - Bonus: no alert to nearby animals (silent kill)
```

#### 5.7.5 Hunting & Harvesting
```
TRACKING:
  - Animals leave footprints visible within 10m (skill increases range)
  - Footprint despawn: 10 min game-time
  - Blood trail when animal is wounded
  - Sound cues: distant animal calls indicate presence

TRAPPING:
  - Snare Trap: catches rabbits/small animals passively (bait: berries)
  - Pit Trap: catches medium animals (dig + cover with branches)
  - Bait Station: attracts animals within 50m over 5 min

HARVESTING (requires knife):
  - Small animal (rabbit):   raw_meat ×1, hide ×1, bone ×1
  - Medium animal (deer):    raw_meat ×4, hide ×2, bone ×3, gut ×1
  - Large animal (bear):     raw_meat ×8, hide ×4, bone ×5, fat ×3, claws ×2
  - Harvest time: 5s (small), 10s (medium), 20s (large)

SPOILAGE:
  - Raw meat: spoils in 15 min game-time
  - Cooked meat: spoils in 60 min game-time
  - Dried meat: spoils in 300 min game-time (drying rack)
  - Refrigerated: no spoilage (requires power)
  - Spoiled food: eating causes Poisoning effect
```

---

### 5.8 SURVIVAL MECHANICS
**Location:** `Source/Core/Survival/`

#### 5.8.1 Temperature System
```
BODY_TEMP CALCULATION (per tick, every 5 seconds):
  body_temp += (target_temp - body_temp) × 0.05

  target_temp = biome_ambient_temp
    + clothing_insulation_total
    + fire_warmth (if within radius)
    + shelter_bonus (if inside building with roof)
    + wind_chill_penalty (exposed × wind_speed × -0.5)
    + wetness_penalty (wet = -10°C equivalent)

CLOTHING INSULATION VALUES:
  - Cloth shirt: +2°C, Leather vest: +5°C, Fur coat: +15°C
  - Cloth pants: +2°C, Leather pants: +4°C, Fur pants: +12°C
  - Sandals: +0°C, Leather boots: +3°C, Fur boots: +8°C
  - Total needed for Arctic survival: ≥ +25°C

FIRE WARMTH:
  - Campfire: +15°C within 3m, +8°C within 5m, +0°C beyond
  - Torch (held): +3°C to player
  - Building with campfire inside: entire interior +15°C

SHELTER BONUS:
  - Enclosed building (walls + roof): +5°C base
  - With door closed: additional +2°C
  - Wind blocked: negates wind_chill_penalty entirely
```

#### 5.8.2 Nutrition System
```
FOOD PROPERTIES:
  Each food item has: hunger_restore, thirst_restore, health_restore,
                      protein, carbs, vitamins, fat, cook_required, spoil_time

EXAMPLES:
  Raw Berries:    hunger=5,  thirst=2,  health=0,  vitamins=3, spoil=30min
  Cooked Meat:    hunger=25, thirst=0,  health=5,  protein=8,  spoil=60min
  Stew:           hunger=40, thirst=10, health=10, protein=5, carbs=5, vitamins=3, spoil=45min
  Dried Meat:     hunger=15, thirst=-5, health=0,  protein=6,  spoil=300min
  Cactus Water:   hunger=0,  thirst=15, health=0,                spoil=never
  Boiled Water:   hunger=0,  thirst=30, health=0,                spoil=never

BALANCED DIET BONUS:
  If in last 24h game-time player consumed ≥ 5 protein + 5 carbs + 3 vitamins + 3 fat:
    → "Well Nourished" buff: +20% stamina regen, +1 health regen/s for 12h game-time
```

#### 5.8.3 Medical System
```
INJURY: Bleeding
  trigger:  animal attack, fall > 5m, sharp weapon hit
  effect:   -3 HP/min per stack (max 3 stacks)
  cure:     Bandage (1 stack), Medical Kit (all stacks)
  auto_heal: never (must be treated)

INJURY: Fracture
  trigger:  fall > 10m, heavy blunt attack, bear attack
  effect:   speed × 0.5, cannot sprint, cannot jump
  cure:     Splint + 10 min rest
  auto_heal: 120 min game-time (with splint)

INJURY: Burn
  trigger:  fire damage, campfire contact, lava (if added)
  effect:   -2 HP/min, pain screen effect
  cure:     Aloe Salve (jungle biome plant), Burn Cream (alchemy)
  auto_heal: 30 min game-time

INJURY: Infection
  trigger:  30 min after untreated bleeding wound
  effect:   -1 HP/min (escalates to -3 HP/min over time), fever (temp +2°C)
  cure:     Antibiotics (alchemy station), Medicinal Herbs (jungle/mountain)
  auto_heal: never (fatal if untreated)

INJURY: Food Poisoning
  trigger:  eating raw/spoiled food, drinking unpurified water
  effect:   -2 HP/min for 5 min, vomiting animation, hunger -20
  cure:     Antidote (alchemy station), or wait it out
  auto_heal: 5 min game-time
```

#### 5.8.4 Sleep & Rest
```
FATIGUE:
  - Fatigue accumulates: 1 point per 1 min game-time
  - Max fatigue: 100
  - At fatigue 80: "Drowsy" → screen dims periodically, stamina regen × 0.5
  - At fatigue 100: "Exhausted" → forced blackout for 10s every 2 min, all stats drain faster

SLEEPING:
  - Bed (in shelter):        fatigue -100, health +20, time skip 8h, comfort +50
  - Sleeping Bag (anywhere): fatigue -60,  health +5,  time skip 6h, comfort +10
  - Ground (no equipment):   fatigue -30,  health +0,  time skip 4h, comfort -10, disease risk

RESPAWN POINTS:
  - Last bed slept in = primary respawn
  - Sleeping bag = one-time respawn (consumed on use)
  - No spawn set = random position in starting biome
```

---

### 5.9 MULTIPLAYER & NETWORKING
**Location:** `Source/Networking/`

#### 5.9.1 Architecture
```
SINGLEPLAYER:
  - Full game runs locally
  - Save to local file (JSON/binary)
  - Multiple save slots (5 minimum)
  - Auto-save every 5 min game-time
  - Manual save via pause menu

LAN MODE:
  - One player acts as host (listen server)
  - Other players on same network join via IP or auto-discovery (UDP broadcast)
  - Player count: 2-8 (configurable by host)
  - Host has admin controls (kick, ban, time of day, weather)
  - World saved on host machine
  - If host disconnects → session ends (with save)

ONLINE CO-OP:
  - Dedicated server model (preferred) or listen server
  - Steam Sockets for NAT traversal
  - Player count: 2-32 (server configurable)
  - Server browser + direct connect
  - Password-protected servers
  - Server-side world persistence
  - Character data: saved per-server

REPLICATION RULES:
  - Player movement: client-predicted, server-authoritative
  - Inventory changes: server-authoritative
  - Building placement: client-requests, server-validates
  - Combat damage: server-calculated
  - Wildlife AI: server-simulated, replicated to clients
  - Day/night + weather: server-driven, synced to all clients
```

#### 5.9.2 Server Modes
```
MODE: PvE (default)
  - No player-vs-player damage
  - Shared building (team permissions)
  - Cooperative survival focus
  - Friendly fire: OFF

MODE: PvPvE
  - Full PvP enabled
  - Base raiding allowed (during raid hours, configurable)
  - Loot drop on death: configurable (none / hotbar / all)
  - Safe zones around fresh spawns (5 min invincibility)
  - Clan/team system for allies
```

#### 5.9.3 Co-op Features
```
TEAMS:
  - Create team (max 8 players)
  - Shared building permissions
  - Team markers on HUD
  - Shared map reveals

TRADING:
  - Direct trade window (both players confirm)
  - No automated trading post (keeps it personal)

VOICE CHAT:
  - Proximity-based: 30m radius
  - Team channel: unlimited range
  - Push-to-talk default

TEXT CHAT:
  - Global, team, and proximity channels
  - Chat log in UI
```

---

### 5.10 UI/UX DESIGN
**Location:** `Source/UI/`

#### 5.10.1 HUD Layout
```
TOP-LEFT:      Mini-map (fog-of-war, reveals as explored)
TOP-CENTER:    Compass bar (N/S/E/W + degree heading)
TOP-RIGHT:     Status effect icons (burning, bleeding, cold, etc.)

BOTTOM-LEFT:   Player stats (vertical bars):
               - Health (red), Hunger (orange), Thirst (blue),
               - Stamina (yellow), Temperature (thermometer icon)

BOTTOM-CENTER: Hotbar (slots 1-8 with item icons + durability)
BOTTOM-RIGHT:  Equipment weight / capacity indicator

CENTER:        Crosshair (context-sensitive: changes for interact, attack, gather)
               Interaction prompt ("E to Gather", "E to Open", etc.)

TOGGLE:        Full HUD / Minimal HUD / No HUD (cycle with H key)
```

#### 5.10.2 Menus
```
MAIN MENU:
  - Singleplayer → New Game / Load Game / Settings
  - Multiplayer → Host Game / Join Game (Server Browser) / Direct Connect
  - Settings → Graphics / Audio / Controls / Gameplay / Accessibility
  - Credits
  - Quit

PAUSE MENU (in-game):
  - Resume
  - Inventory (also: TAB shortcut)
  - Crafting (also: C shortcut)
  - Map (also: M shortcut)
  - Journal (recipes discovered, quest log)
  - Settings
  - Save Game (singleplayer only)
  - Quit to Main Menu

SETTINGS:
  Graphics: Resolution, Window Mode, Quality Preset (Low/Med/High/Ultra/Custom),
            V-Sync, Frame Rate Cap, FOV Slider, View Distance,
            Shadow Quality, Texture Quality, Anti-Aliasing, Post-Processing
  Audio:    Master Volume, Music, SFX, Ambient, Voice Chat, Subtitles Toggle
  Controls: Keybinds (full remap), Mouse Sensitivity, Invert Y, Controller Support
  Gameplay: HUD Mode, Crosshair Style, Damage Numbers, Auto-Pickup
  Access:   Colorblind Mode (Deuteranopia/Protanopia/Tritanopia), Text Size, Narrator
```

---

### 5.11 DAY/NIGHT & WEATHER
**Location:** `Source/World/Weather/` and `Source/World/DayNightCycle/`

```
DAY/NIGHT CYCLE:
  - 1 game-day = 24 real-time minutes
  - Sunrise: 06:00 game-time (minute 6 real-time)
  - Sunset: 18:00 game-time (minute 18 real-time)
  - Darkest: 00:00 game-time
  - Light level affects: visibility, wildlife behavior, temperature, stealth

WEATHER SYSTEM:
  - Weather changes every 30-90 min game-time (random within biome weights)
  - Weather affects: temperature, visibility, movement speed, fire (rain extinguishes)

WEATHER EVENTS:
  Rain:        temp -3°C, fire extinguish (outdoor), visibility -20%, wet status
  Heavy Rain:  temp -5°C, fire extinguish, visibility -50%, wet status, flood risk (swamp)
  Snow:        temp -8°C, movement speed -10% (snow accumulation), visibility -30%
  Blizzard:    temp -15°C, movement -30%, visibility -80%, damage 1HP/min if unsheltered
  Sandstorm:   visibility -90%, damage 2HP/min if unsheltered, movement -20%
  Fog:         visibility -60%, no temp effect, stealth bonus for all
  Thunderstorm: rain effects + lightning strikes (rare, can ignite trees/structures)
  Heat Wave:   temp +10°C, thirst drain ×2 (desert only)
  Wildfire:    spreads through dry grass, destroys wooden structures, forces evacuation
  Tornado:     RARE event, destroys structures in path, throws objects (savanna/grassland)
  Avalanche:   RARE event, triggered by noise or weather, damages anything in path (mountain)
```

---

### 5.12 SKILL & PROGRESSION SYSTEM
**Location:** `Source/Core/Player/Stats/`

```
SKILL SYSTEM: Skills improve by performing related actions (no XP points to spend).

SKILL: Gathering (levels 1-100)
  - Each harvest action grants gathering_xp
  - Higher level → more yield per harvest, faster gather speed
  - Level thresholds: 10=stone tools efficient, 30=iron tools efficient, 60=rare resources visible

SKILL: Crafting (levels 1-100)
  - Each craft action grants crafting_xp
  - Higher level → better quality items, less material waste, faster craft time
  - Level thresholds: 20=advanced recipes, 50=rare recipes, 80=master recipes

SKILL: Combat (levels 1-100)
  - Each attack/kill grants combat_xp
  - Higher level → more melee damage, faster attack speed, better crit chance
  - Level thresholds: 15=combo attacks, 40=power attacks, 70=execution moves

SKILL: Survival (levels 1-100)
  - Passive XP from staying alive, treating injuries, enduring weather
  - Higher level → slower hunger/thirst drain, better temperature resistance
  - Level thresholds: 10=status effect resist, 30=efficient digestion, 60=iron stomach

SKILL: Stealth (levels 1-100)
  - XP from undetected movement, stealth kills
  - Higher level → reduced detection radius, quieter movement, faster stealth kill
  - Level thresholds: 15=crouch speed++, 40=silent sprint, 70=invisible prone in foliage

SKILL: Building (levels 1-100)
  - XP from placing/upgrading structures
  - Higher level → stronger structures (+HP%), more piece variety, faster placement
  - Level thresholds: 10=stone tier, 30=metal tier, 50=power system, 80=advanced automation
```

---

### 5.13 TRANSPORTATION
```
MOUNT: Horse (Temperate Woods, Savanna)
  - Tame with: approach slowly + offer apples/carrots × 3
  - Speed: 800 cm/s (walk), 1200 cm/s (gallop)
  - Stamina: separate bar, regens when idle
  - Can carry player + saddlebag (extra 20 kg capacity)

MOUNT: Camel (Desert)
  - Tame with: approach slowly + offer cactus fruit × 5
  - Speed: 600 cm/s (walk), 900 cm/s (gallop)
  - Special: reduced water consumption while riding

BOAT: Raft
  - Craft at: Workbench (15 wood, 10 rope)
  - Speed: 300 cm/s, holds 1-2 players
  - Slow, no cargo space

BOAT: Canoe
  - Craft at: Workbench (30 wood, 10 rope, 5 leather)
  - Speed: 500 cm/s, holds 2 players + storage

BOAT: Sailboat
  - Craft at: Workbench (80 wood, 20 rope, 10 cloth, 5 iron)
  - Speed: 700 cm/s (wind-dependent), holds 4 players + large storage
```

---

## 6. Development Phases — Detailed Roadmap

### PHASE 0: Pre-Production (Months 0-2)
```
TASKS:
  □ Finalize this plan document
  □ Create Unreal Engine 5.4 project with folder structure
  □ Configure Git + LFS for project
  □ Download free asset packs (see Section 2.1)
  □ Set up Visual Studio 2022 with UE5 integration
  □ Create base C++ classes: GameMode, PlayerCharacter, GameState
  □ Import placeholder character model (from Mixamo)
  □ Import placeholder terrain textures (from Poly Haven)
  □ Create initial landscape (flat test level)
DELIVERABLE: Empty project builds and runs, player can walk on flat terrain
```

### PHASE 1: Player Foundation (Months 2-4)
```
TASKS:
  □ Implement CharacterMovementComponent extensions
    - Walk/Run/Sprint/Crouch/Prone state machine
    - Stamina system
    - Swimming detection and swim mode
    - Climbing (trace-based surface detection)
    - Slope sliding
  □ Implement Camera system (FPS/TPS toggle)
  □ Implement Player Stats component (health, hunger, thirst, temp, stamina)
  □ Implement Status Effects system (buff/debuff framework)
  □ Implement basic interaction system (trace + interact key)
  □ Create HUD (stat bars, crosshair, interaction prompts)
  □ Set up input mapping (keyboard + mouse + gamepad)
DELIVERABLE: Player can move, sprint, jump, swim, and has draining stats
```

### PHASE 2: Inventory & Resources (Months 4-6)
```
TASKS:
  □ Implement InventoryComponent (grid-based)
  □ Create ItemDefinition DataTable structure
  □ Populate initial item database (20+ basic items)
  □ Implement inventory UI (grid, drag-drop, tooltips, hotbar)
  □ Implement equipment slots and stat modifiers from gear
  □ Implement ResourceNode base class (harvestable objects)
  □ Create resource nodes: trees, rocks, bushes, ore deposits
  □ Implement gathering system (tool check, yield calculation, node HP)
  □ Implement item pickup / drop in world
  □ Implement container system (chests)
DELIVERABLE: Player can gather resources, manage inventory, equip items
```

### PHASE 3: Crafting (Months 6-8)
```
TASKS:
  □ Implement CraftingComponent
  □ Create Recipe DataTable structure (JSON-importable)
  □ Populate initial recipes (30+ recipes across categories)
  □ Implement crafting UI (recipe browser, ingredient check, craft button)
  □ Implement crafting stations (placeable world objects)
  □ Implement campfire (cooking + warmth)
  □ Implement workbench, forge, tanning rack
  □ Implement item durability and repair system
  □ Implement recipe discovery system
DELIVERABLE: Full crafting loop — gather → craft tool → gather better → craft station → craft advanced items
```

### PHASE 4: Building (Months 8-11)
```
TASKS:
  □ Implement BuildingComponent (enter/exit build mode)
  □ Create building piece blueprint classes (foundation, wall, floor, roof, etc.)
  □ Implement snap system (socket-based snapping)
  □ Implement ghost preview with validity checking
  □ Implement 3 material tiers (thatch/wood, stone, metal)
  □ Implement upgrade system (tier 1 → 2 → 3)
  □ Implement structure HP and damage
  □ Implement functional structures (bed, campfire, storage, etc.)
  □ Implement farming (planter box, crop growth over time)
  □ Implement building permissions (ownership, team sharing)
DELIVERABLE: Player can build multi-tier bases with functional interiors
```

### PHASE 5: World & Biomes (Months 11-15)
```
TASKS:
  □ Create world landscape (hand-crafted heightmap, ~8 km² total)
  □ Define 7 biome regions on landscape
  □ Create landscape material with per-biome layers
  □ Populate each biome with vegetation (UE Megascans free + Quaternius)
  □ Implement biome transition blending
  □ Implement day/night cycle (sun movement, sky colors, lighting)
  □ Implement weather system (per-biome weather tables)
  □ Implement temperature system (biome + weather + clothing + shelter)
  □ Place resource nodes per biome (density tables)
  □ Add water bodies (rivers, lakes, ocean edges)
  □ Add caves and points of interest per biome
DELIVERABLE: Complete open world with 7 unique biomes, weather, and day/night
```

### PHASE 6: Wildlife & Combat (Months 15-18)
```
TASKS:
  □ Create Animal base class (stats, loot table, behavior tree)
  □ Implement behavior trees (passive, neutral, aggressive, territorial, pack)
  □ Create 25+ animal species with per-biome spawn tables
  □ Import animal models (Quaternius CC0 pack or UE free assets)
  □ Implement animal animations (idle, walk, run, attack, death)
  □ Implement melee combat (light/heavy attack, combo, block, parry, dodge)
  □ Implement ranged combat (bow, throwables, projectile physics)
  □ Implement damage types and armor system
  □ Implement stealth system (detection model)
  □ Implement hunting loop (track, kill, harvest, cook)
  □ Implement trapping system (snare, pit, bait)
  □ Implement animal spawning and despawning (performance-managed)
DELIVERABLE: Living world with dangerous wildlife, full combat, and hunting
```

### PHASE 7: Survival Polish (Months 18-20)
```
TASKS:
  □ Finalize temperature system (all biomes balanced)
  □ Implement nutrition system (food types, balanced diet buff)
  □ Implement full medical system (bleeding, fracture, infection, treatment)
  □ Implement sleep/fatigue system (beds, sleeping bags, rest quality)
  □ Implement food spoilage system (timers, preservation methods)
  □ Implement water purification chain
  □ Implement clothing insulation values (balance for each biome)
  □ Add skill/progression system
  □ Add transportation (mounts, boats)
  □ Balance all survival values across biomes
DELIVERABLE: Complete survival simulation — every biome is a unique challenge
```

### PHASE 8: Multiplayer (Months 20-25)
```
TASKS:
  □ Convert all game systems to replicated (UE5 replication framework)
  □ Implement server-authoritative inventory, crafting, building
  □ Implement client-predicted movement with server correction
  □ Implement singleplayer save/load system
  □ Implement LAN host/join (listen server + UDP discovery)
  □ Implement online dedicated server
  □ Implement Steam Sockets integration (NAT traversal)
  □ Implement server browser UI
  □ Implement team/clan system
  □ Implement proximity voice chat
  □ Implement text chat (global, team, proximity)
  □ Implement player trading system
  □ Implement PvE and PvPvE modes
  □ Implement admin commands (kick, ban, weather, time)
  □ Stress test with 32 players
DELIVERABLE: Seamless singleplayer ↔ multiplayer, LAN and online co-op working
```

### PHASE 9: Polish & Launch Prep (Months 25-30)
```
TASKS:
  □ Performance optimization (LOD, occlusion, draw call batching, streaming)
  □ Target: 60 FPS stable on recommended specs
  □ Implement all weather VFX (particles, post-process)
  □ Implement dynamic music system (calm → tension → combat)
  □ Add ambient soundscapes per biome
  □ Finalize all UI screens (main menu, settings, server browser)
  □ Implement accessibility (colorblind modes, text size, remappable controls)
  □ Add tutorial / guided first hour
  □ Add achievements / journal system
  □ Replace placeholder assets with final art (highest priority items)
  □ Bug fixing and balance passes
  □ Closed beta test (community feedback)
  □ Open beta test
  □ Prepare Steam store page, trailer, marketing materials
  □ Final certification and release build
DELIVERABLE: Version 1.0 shipped on Steam
```

---

## 7. Team Structure

### Solo / Small Indie (1-3 people)
```
Developer 1: C++ Gameplay Programming, Systems Design
Developer 2: Level Design, Environment Art, 3D Modeling (Blender)
Developer 3: UI/UX, Audio, QA Testing (can be part-time/contract)
```

### Recommended Studio (8-12 people)
```
1× Creative Director / Game Designer
3× Programmers (Gameplay, Networking, AI)
2× Environment Artists
1× Character/Creature Artist
1× Animator
1× UI/UX Designer
1× Audio Designer
1× QA Tester
1× Producer / Project Manager
```

---

## 8. Budget Overview

| Category | Indie ($) | Studio ($) |
|----------|-----------|------------|
| Salaries/Contracts (30 mo) | 200k–400k | 600k–1.2M |
| Hardware | 10k–20k | 30k–60k |
| Software Licenses | 0–5k (mostly free) | 5k–15k |
| Free Assets + Commissions | 0–10k | 20k–50k |
| Audio/Music | 0–5k | 10k–25k |
| Marketing | 5k–20k | 30k–80k |
| Server Infrastructure (/yr) | 3k–10k | 15k–40k |
| **TOTAL** | **~220k–470k** | **~710k–1.5M** |

---

## 9. Monetization

```
MODEL: Premium (buy-to-play)
  Base Game:       $24.99 – $34.99 USD
  DLC Expansions:  $9.99 – $14.99 (new biomes, story, creatures)
  Cosmetic Packs:  $2.99 – $7.99 (skins, emotes — no gameplay advantage)

NEVER: Pay-to-win mechanics. All gameplay items are earnable in-game.
```

---

## 10. Risk Mitigation

| Risk | Impact | Mitigation |
|------|--------|------------|
| Scope creep | High | Strict phase milestones, MVP-first approach |
| Multiplayer sync bugs | High | Use proven UE5 replication, test early & often |
| Performance (open world) | High | LOD, streaming, occlusion culling, profiling from Phase 5 |
| Asset quality gap | Medium | Start free → replace incrementally, art style covers gaps |
| Market saturation | Medium | Unique multi-biome USP, strong community engagement |
| Team burnout | Medium | Realistic timeline, buffer months, work-life balance |

---

## 11. Success Metrics

| Metric | Target |
|--------|--------|
| Crash rate | < 1% |
| FPS (recommended specs) | ≥ 60 stable |
| FPS (minimum specs) | ≥ 30 stable |
| Multiplayer latency | < 100ms |
| Server uptime | > 99% |
| Steam review score | > 75% positive |
| Units sold (month 1) | > 10,000 |
| Player retention (7-day) | > 40% |
| Peak concurrent players | > 5,000 |

---

## 12. Post-Launch Roadmap

```
Month 1-3:  Weekly hotfix patches, community feedback integration
Month 3-6:  Content Update 1 — new animals, recipes, building pieces
Month 6-9:  Content Update 2 — new biome (Volcanic / Underwater cave)
Month 9-12: DLC 1 — story mode, NPC encounters, quest system
Year 2:     Modding support (Steam Workshop), community-created content
Year 2+:    DLC 2 — new map, seasons system, vehicle expansion
```

---

## Appendix: Key Conventions for AI Code Generation

<!-- AI-INSTRUCTION: When generating code for this project, follow these rules: -->

```
NAMING CONVENTIONS:
  - C++ classes:    PascalCase with prefix (APlayerCharacter, UInventoryComponent)
  - Functions:      PascalCase (GetCurrentHealth, AddItemToInventory)
  - Variables:      camelCase (currentHealth, maxStamina)
  - Constants:      UPPER_SNAKE_CASE (MAX_INVENTORY_SLOTS, DEFAULT_WALK_SPEED)
  - Config files:   snake_case.json (item_database.json, crafting_recipes.json)
  - Assets:         PascalCase (SM_TreeOak_01, T_Grass_Normal, M_Water_Lake)

UE5 PATTERNS:
  - Use UActorComponent for systems (InventoryComponent, CraftingComponent)
  - Use ACharacter for player (APlayerCharacter)
  - Use AActor for world objects (AResourceNode, ABuildingPiece)
  - Use UDataTable for item/recipe databases
  - Use UGameInstance for persistent data across levels
  - Use AGameMode for server-side game rules
  - Use AGameState for replicated world state
  - Use APlayerState for per-player replicated data
  - Use Gameplay Ability System (GAS) for status effects

FILE ORGANIZATION:
  - One class per .h/.cpp pair
  - Headers in Public/, implementations in Private/
  - Group by system (Inventory/, Crafting/, Combat/, etc.)
  - Config data in DataTables or JSON (not hardcoded)
```

---

**Document Version:** 2.0  
**Last Updated:** 2026-03-05  
**Status:** Pre-Production  
**Next Action:** Set up UE5 project and implement Phase 0

---

*This is the master plan. All implementation work references this document.*
