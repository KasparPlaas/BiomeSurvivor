# Software Requirements — Biome Survivor

All software needed to develop **Biome Survivor**, with download links, versions, and installation notes.

---

## Table of Contents

- [Required Software](#required-software)
- [Optional / Recommended Software](#optional--recommended-software)
- [Free Asset Sources](#free-asset-sources)
- [Hardware Requirements](#hardware-requirements)
- [Installation Guide (Step-by-Step)](#installation-guide-step-by-step)
- [Verification Checklist](#verification-checklist)

---

## Required Software

These tools **must** be installed before development can begin.

### 1. Operating System — Windows 11 Professional

| | |
|---|---|
| **Version** | Windows 11 Professional 64-bit |
| **Download** | https://www.microsoft.com/en-us/windows/get-windows-11 |
| **Cost** | $199.99 (or included with PC) |
| **Why** | Best UE5 compatibility, Visual Studio integration, DirectX 12, GPU driver support |

> Windows 10 64-bit also works but Windows 11 is recommended for latest features.

---

### 2. Game Engine — Unreal Engine 5.4+

| | |
|---|---|
| **Version** | 5.4 or newer |
| **Download** | https://www.unrealengine.com/download |
| **Launcher** | Epic Games Launcher (required to install UE) |
| **Cost** | **FREE** (5% royalty after first $1,000,000 in revenue) |
| **Size** | ~50-60 GB installed |

**Installation Steps:**
1. Go to https://www.unrealengine.com/download
2. Click "Download Now" → installs **Epic Games Launcher**
3. Open Epic Games Launcher → go to **Unreal Engine** tab → **Library**
4. Click **＋** next to "Engine Versions" → select **5.4.x**
5. Click ▼ next to the version → **Options** → ensure these are checked:
   - ✅ Core Components
   - ✅ Starter Content
   - ✅ Templates and Feature Packs
   - ✅ Engine Source (optional but recommended)
6. Click **Install** and wait for completion

**Required Components during install:**
- Target Platforms: Windows
- Prerequisite Installer

---

### 3. IDE — Visual Studio 2022 Community

| | |
|---|---|
| **Version** | 2022 Community (17.x) |
| **Download** | https://visualstudio.microsoft.com/vs/community/ |
| **Direct download** | https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022 |
| **Cost** | **FREE** (for individuals, open source, education, and up to 5-user companies) |
| **Size** | ~8-30 GB depending on workloads |

**Required Workloads (select during installation):**
- ✅ **Desktop development with C++**
- ✅ **Game development with C++**
  - ✅ Unreal Engine installer (sub-option)
- ✅ **.NET desktop development** (for some UE5 tooling)

**Required Individual Components:**
- ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
- ✅ Windows 11 SDK (10.0.22621.0 or newer)
- ✅ C++ CMake tools for Windows
- ✅ C++ ATL for latest build tools

**Post-Install Configuration:**
1. Open Visual Studio → Tools → Options → Text Editor → C/C++ → Formatting
2. Enable "Format on paste" and "Format on save" for consistent code style
3. Install extension: **UnrealVS** (included with UE5 installation)
   - Located at: `C:\Program Files\Epic Games\UE_5.4\Engine\Extras\UnrealVS\`

---

### 4. Version Control — Git + Git LFS

| | |
|---|---|
| **Git Version** | Latest (2.43+) |
| **Download Git** | https://git-scm.com/download/win |
| **Git LFS** | Latest |
| **Download Git LFS** | https://git-lfs.github.com/ |
| **Cost** | **FREE** |

**Installation Steps:**
1. Download and install Git from https://git-scm.com/download/win
   - During setup: select "Git from the command line and also from 3rd-party software"
   - Line ending: "Checkout Windows-style, commit Unix-style line endings"
   - Default editor: Visual Studio Code or your preferred editor
2. Download and install Git LFS from https://git-lfs.github.com/
3. Open terminal and run:
```bash
git lfs install
```
4. Configure your identity:
```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

**Why Git LFS?**
Unreal Engine projects contain large binary files (.uasset, .umap, textures, audio). Git LFS stores these efficiently without bloating the repository.

---

### 5. Remote Repository — GitHub or GitLab

| | |
|---|---|
| **GitHub** | https://github.com/ |
| **GitLab** | https://gitlab.com/ |
| **Cost** | **FREE** (free tier includes private repos with collaborators) |

**Recommended:** GitHub + GitHub LFS
- Free tier: 1 GB LFS storage, 1 GB bandwidth/month
- If you need more: GitHub Team ($4/user/month) includes 2 GB LFS

---

## Optional / Recommended Software

These tools are highly recommended but not blocking for initial development.

### 6. 3D Modeling — Blender

| | |
|---|---|
| **Version** | 4.0+ (latest stable) |
| **Download** | https://www.blender.org/download/ |
| **Cost** | **FREE** (open source, GPL) |
| **Size** | ~350 MB installed |
| **Purpose** | 3D modeling, sculpting, UV unwrapping, animation, rendering |

**Recommended Addons:**
- **Send to Unreal** (Blender → UE5 export): https://github.com/EpicGames/BlenderTools
- **Rigify** (character rigging — built-in, enable in Preferences → Add-ons)

---

### 7. 2D Image Editing — GIMP

| | |
|---|---|
| **Version** | 2.10+ |
| **Download** | https://www.gimp.org/downloads/ |
| **Cost** | **FREE** (open source, GPL) |
| **Size** | ~300 MB installed |
| **Purpose** | Texture editing, icon creation, UI element design |

**Alternative (also free):** Krita — https://krita.org/en/download/krita-desktop/ (better for painting/drawing)

---

### 8. Audio Editing — Audacity

| | |
|---|---|
| **Version** | 3.4+ |
| **Download** | https://www.audacityteam.org/download/ |
| **Cost** | **FREE** (open source, GPL) |
| **Size** | ~100 MB installed |
| **Purpose** | Recording, editing, and converting audio files (SFX, music) |

---

### 9. Audio Middleware — FMOD Studio

| | |
|---|---|
| **Version** | Latest (2.02+) |
| **Download** | https://www.fmod.com/download |
| **UE Plugin** | https://www.fmod.com/docs/2.02/unreal/welcome.html |
| **Cost** | **FREE** for indie (budget < $200K) |
| **Size** | ~500 MB installed |
| **Purpose** | Adaptive music, 3D audio, sound events, biome ambience |

**Setup:**
1. Download FMOD Studio from https://www.fmod.com/download (requires free account)
2. Download FMOD UE5 Plugin from the same page
3. Extract plugin to your project's `Plugins/` folder
4. Enable in UE5: Edit → Plugins → search "FMOD"

---

### 10. Texturing — Substance Alternatives (Free)

If you can't afford Substance Painter/Designer, use these free alternatives:

| Tool | Download | Purpose |
|------|----------|---------|
| **ArmorPaint** | https://armorpaint.org/download.html | Free 3D texture painting (like Substance Painter) |
| **Material Maker** | https://www.materialmaker.org/ | Free procedural material creation (like Substance Designer) |
| **Quixel Mixer** | https://quixel.com/mixer | Free material blending (by Epic Games) |

---

### 11. Project Management — Trello / Notion

| Tool | URL | Cost |
|------|-----|------|
| **Trello** | https://trello.com/ | **FREE** (basic board) |
| **Notion** | https://www.notion.so/ | **FREE** (personal use) |
| **GitHub Projects** | https://github.com/features/issues | **FREE** (with GitHub repo) |

Use one of these to track tasks per development phase.

---

### 12. Communication — Discord

| | |
|---|---|
| **Download** | https://discord.com/download |
| **Cost** | **FREE** |
| **Purpose** | Team communication, voice chat, file sharing, community building |

Create a private Discord server for your dev team and a public one for community building pre-launch.

---

### 13. Screen Recording / Streaming — OBS Studio

| | |
|---|---|
| **Download** | https://obsproject.com/download |
| **Cost** | **FREE** (open source) |
| **Purpose** | Record dev logs, capture gameplay footage, stream development |

---

## Free Asset Sources

All assets below are **free to use** in commercial projects (check individual licenses).

### 3D Models & Props

| Source | URL | License | Description |
|--------|-----|---------|-------------|
| **UE Marketplace (Free)** | https://www.unrealengine.com/marketplace/en-US/assets?count=20&priceRange=%5B0%2C0%5D&sortBy=effectiveDate&sortDir=DESC | Epic EULA | Monthly free packs + permanently free collection |
| **Quixel Megascans** | https://quixel.com/megascans/free | Free with UE5 | Photoscanned assets — rocks, vegetation, surfaces |
| **Quaternius** | https://quaternius.com/packs.html | CC0 | Low-poly animals, nature, buildings, characters |
| **Kenney.nl** | https://kenney.nl/assets | CC0 | Game-ready asset packs (many categories) |
| **Poly Haven Models** | https://polyhaven.com/models | CC0 | Realistic models |
| **Sketchfab (CC0)** | https://sketchfab.com/search?licenses=7c23a1ba438d4306920229c12afcb5f9&type=models | CC0/CC-BY | Thousands of free 3D models |
| **Turbosquid (Free)** | https://www.turbosquid.com/Search/3D-Models/free | Various | Free models (check license per asset) |
| **OpenGameArt 3D** | https://opengameart.org/art-search-advanced?keys=&field_art_type_tid%5B%5D=10 | CC0/CC-BY | Community game art |

### Textures & Materials

| Source | URL | License | Description |
|--------|-----|---------|-------------|
| **Poly Haven Textures** | https://polyhaven.com/textures | CC0 | 4K PBR texture sets |
| **ambientCG** | https://ambientcg.com/ | CC0 | PBR materials, decals, HDRIs |
| **Texture Ninja** | https://texture.ninja/ | Free use | High-res photo textures |
| **3D Textures** | https://3dtextures.me/ | CC0 | Seamless PBR textures |
| **Poly Haven HDRIs** | https://polyhaven.com/hdris | CC0 | Environment maps for lighting |

### Animations

| Source | URL | License | Description |
|--------|-----|---------|-------------|
| **Mixamo** | https://www.mixamo.com/ | Free (Adobe) | Character animations + auto-rigging |
| **ActorCore (free)** | https://actorcore.reallusion.com/ | Various | Motion capture animations |

### Audio & Music

| Source | URL | License | Description |
|--------|-----|---------|-------------|
| **Freesound.org** | https://freesound.org/ | CC0/CC-BY | Massive SFX library |
| **Sonniss GDC Bundle** | https://sonniss.com/gameaudiogdc | Royalty-free | Annual free SFX bundles (30+ GB) |
| **OpenGameArt Audio** | https://opengameart.org/art-search-advanced?keys=&field_art_type_tid%5B%5D=13 | CC0/CC-BY | Game music & SFX |
| **Incompetech** | https://incompetech.com/music/ | CC-BY | Royalty-free music by Kevin MacLeod |
| **FreePD** | https://freepd.com/ | CC0 | Public domain music |
| **Pixabay Music** | https://pixabay.com/music/ | Pixabay License | Free music for any use |

### UI Assets

| Source | URL | License | Description |
|--------|-----|---------|-------------|
| **Game-icons.net** | https://game-icons.net/ | CC-BY 3.0 | 4000+ game icons (swords, potions, etc.) |
| **Google Fonts** | https://fonts.google.com/ | Open Font License | Thousands of free fonts |
| **Font Awesome** | https://fontawesome.com/icons?d=gallery&m=free | CC-BY/SIL OFL | UI icons |
| **Google Material Icons** | https://fonts.google.com/icons | Apache 2.0 | Clean UI icons |

---

## Hardware Requirements

### Development Machine (Recommended)

| Component | Specification | Approx. Cost |
|-----------|--------------|--------------|
| **CPU** | Intel Core i9-12900K / AMD Ryzen 9 5950X | $400-550 |
| **RAM** | 64 GB DDR5 (or DDR4-3600) | $150-250 |
| **GPU** | NVIDIA RTX 4070 Ti (16 GB VRAM) | $700-800 |
| **Storage** | 1 TB NVMe SSD (project) + 2 TB SSD (assets) | $100-200 |
| **Monitor** | 2560×1440, 27"+ (IPS recommended) | $250-400 |
| **Peripherals** | Mouse, keyboard, headset with mic | $100-200 |
| **Total** | | **~$1,700-2,400** |

### Minimum Development Machine (Budget)

| Component | Specification |
|-----------|--------------|
| **CPU** | Intel Core i7-10700K / AMD Ryzen 7 3700X |
| **RAM** | 32 GB |
| **GPU** | NVIDIA RTX 3060 12 GB |
| **Storage** | 512 GB NVMe SSD + 1 TB HDD |

### Player Target Specs (defined in plan.md)
- **Minimum:** i5-8400, 12 GB RAM, GTX 1060 6 GB
- **Recommended:** i7-10700K, 16 GB RAM, RTX 3060 Ti

---

## Installation Guide (Step-by-Step)

Complete these steps **in this order** to set up a new development environment.

### Step 1: Operating System
- [ ] Install/update Windows 11 Professional 64-bit
- [ ] Run Windows Update until fully current
- [ ] Install latest GPU drivers:
  - NVIDIA: https://www.nvidia.com/download/index.aspx
  - AMD: https://www.amd.com/en/support

### Step 2: Visual Studio 2022
- [ ] Download: https://visualstudio.microsoft.com/vs/community/
- [ ] Install with workloads: Desktop C++, Game C++, .NET Desktop
- [ ] Install individual components: Windows SDK, MSVC build tools, C++ CMake
- [ ] Restart PC after installation

### Step 3: Unreal Engine 5.4+
- [ ] Download Epic Games Launcher: https://www.unrealengine.com/download
- [ ] Create/login to Epic Games account
- [ ] Install UE 5.4+ from Library tab
- [ ] Verify installation by creating a blank project

### Step 4: Git + Git LFS
- [ ] Download Git: https://git-scm.com/download/win
- [ ] Install with default options
- [ ] Download Git LFS: https://git-lfs.github.com/
- [ ] Run `git lfs install` in terminal
- [ ] Configure name and email

### Step 5: Clone Project
- [ ] Clone repository: `git clone <repo-url>`
- [ ] Run `git lfs pull` to download large files
- [ ] Right-click .uproject → "Generate Visual Studio project files"
- [ ] Open .sln in Visual Studio, build, and run

### Step 6: Optional Tools (as needed)
- [ ] Blender 4.0+: https://www.blender.org/download/
- [ ] GIMP 2.10+: https://www.gimp.org/downloads/
- [ ] Audacity: https://www.audacityteam.org/download/
- [ ] FMOD Studio: https://www.fmod.com/download
- [ ] Discord: https://discord.com/download
- [ ] Trello/Notion for task tracking

---

## Verification Checklist

After completing setup, verify everything works:

| # | Test | Expected Result |
|---|------|-----------------|
| 1 | Open command prompt → `git --version` | Shows git version 2.43+ |
| 2 | Run `git lfs version` | Shows git-lfs version |
| 3 | Open Visual Studio → create C++ console app → build | Compiles successfully |
| 4 | Open Epic Games Launcher → launch UE 5.4 | Editor opens without errors |
| 5 | Create blank UE5 C++ project → build | Project compiles and runs |
| 6 | Open Blender → File → Export → FBX | FBX export works |
| 7 | Import FBX into UE5 test project | Model appears in Content Browser |

Once all checks pass, you're ready to start development! Return to [plan.md](plan.md) Phase 0 for your first tasks.

---

**Last Updated:** 2026-03-05  
**Document Version:** 2.0
