# SimpleColorKeyer

A precise color keyer for Foundry Nuke with intuitive 6-direction color expansion control.

![Nuke](https://img.shields.io/badge/Nuke-14.1%2B-yellow)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

## Overview

SimpleColorKeyer provides intelligent color keying with granular control over how the matte expands or contracts along six color directions (Red, Green, Blue, Yellow, Magenta, Cyan). Unlike traditional keyers that only offer global tolerance adjustments, this plugin lets you precisely shape your key by pushing or pulling in specific color directions.

## Features

- **6-Direction Color Control** — Independently expand or contract the key toward/away from any primary or secondary color
- **Multiple Keying Methods** — Distance, Chroma, Luma Weighted, and Adaptive algorithms
- **Intuitive Range Controls** — Each color direction ranges from -3 to +3 for extreme precision
- **Real-time Performance** — Optimized CPU implementation for responsive feedback
- **Production Ready** — Clean alpha output with gain and invert controls

## Installation

### Compiled Plugin

1. Download the appropriate `.so` (Linux), `.dylib` (macOS), or `.dll` (Windows) from the [Releases](https://github.com/petermercell/SimpleColorKeyer/releases) page
2. Copy to your Nuke plugin path:
   - **Linux/macOS**: `~/.nuke/` or a custom plugin directory
   - **Windows**: `C:\Users\<username>\.nuke\` or custom path
3. Restart Nuke

## Usage

Access the node via **Tab → Keyer → SimpleColorKeyer** or search for "SimpleColorKeyer" in the node menu.

### Basic Workflow

1. **Pick your key color** using the eyedropper on the Key Color control
2. **Adjust Tolerance** to set the base matching range
3. **Fine-tune with 6-direction controls** to expand or contract specific color areas
4. **Choose a Keying Method** based on your footage characteristics

### Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Key Color** | RGB | The base color to key out |
| **Tolerance** | 0.001–2.0 | Overall color matching tolerance |
| **Keying Method** | Enum | Algorithm selection (see below) |
| **Gain** | 0.0–5.0 | Alpha contrast multiplier |
| **Invert** | Boolean | Invert the generated matte |

### 6-Direction Color Expansion

Each direction ranges from **-3** to **+3**:
- **Positive values (+)**: Expand the key toward that color
- **Negative values (-)**: Contract the key away from that color

| Direction | Color Space | Typical Use Case |
|-----------|-------------|------------------|
| **Red** | Primary | Isolate warm tones, skin |
| **Green** | Primary | Green screen work |
| **Blue** | Primary | Blue screen, sky |
| **Yellow** | Secondary (R+G) | Foliage, warm spill |
| **Magenta** | Secondary (R+B) | Skin undertones |
| **Cyan** | Secondary (G+B) | Cool shadows, water |

### Keying Methods

| Method | Best For |
|--------|----------|
| **Distance** | General purpose, works with color expansion controls |
| **Chroma** | Footage with lighting variations (ignores luminance) |
| **Luma Weighted** | When brightness similarity matters |
| **Adaptive** | Automatic selection based on key color saturation |

## Examples

### Green Screen with Yellow Spill
```
Key Color: Pick the green screen
Tolerance: 0.3
Green: +1.5
Yellow: +1.0
```

### Blue Screen with Cyan Cast
```
Key Color: Pick the blue screen
Tolerance: 0.35
Blue: +2.0
Cyan: +1.0
```

### Isolate Red Object, Avoid Orange
```
Key Color: Pick the red
Tolerance: 0.25
Red: +1.0
Yellow: -0.5
```

### Skin Tone Selection (Warm Variant)
```
Key Color: Pick skin midtone
Tolerance: 0.4
Red: +0.8
Magenta: +0.3
Yellow: +0.5
```

## Compatibility

- Nuke 14.1 and later
- Windows, Linux, macOS
- Works with all color spaces (operate in linear for best results)

## License

MIT License — see [LICENSE](LICENSE) for details.

## Author

**Peter Mercell**  
[www.petermercell.com](https://www.petermercell.com)

---

*SimpleColorKeyer v2.0 — © 2025 Peter Mercell*
