# Data Directory

This directory contains map files and graph caches for the routing application.

## Structure

```
data/
├── maps/           # OSM map files
│   └── .gitkeep   # (OSM files are excluded from Git due to size)
└── graphs/         # Binary graph caches
    └── .gitkeep   # (Binary caches are regenerated automatically)
```

## Maps Directory (`maps/`)

Place your OSM (OpenStreetMap) XML files here.

**Example:**
- `arequipa.osm`
- `arequipa_big.osm`


## Graphs Directory (`graphs/`)

Binary graph caches are automatically generated from OSM files for faster loading.

**Example:**
- `arequipa.bin` (generated from `arequipa.osm`)
- `arequipa_big.bin` (generated from `arequipa_big.osm`)

**Loading Behavior:**
1. If `.bin` file exists → loads from binary (fast, ~2-4 seconds)
2. If only `.osm` exists → parses OSM and generates `.bin` (slower, ~30-60 seconds)

**Note:** Binary caches are excluded from Git because they can be regenerated from OSM files and may be platform-specific.

## Adding Your Own Maps

1. Download OSM data from:
   - https://www.openstreetmap.org/export
   - https://download.geofabrik.de/
   - Overpass API

2. Place the `.osm` file in `data/maps/`

3. Run the application - it will auto-generate the `.bin` cache


## For Developers

To preserve this directory structure in Git, `.gitkeep` files are included. These are empty placeholder files that force Git to track otherwise empty directories.
