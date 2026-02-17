#!/usr/bin/env python3
"""PNG to RGB565 PROGMEM header converter for TamaFi sprites.

Usage:
    python3 sprite_converter.py input.png --name mySprite --out sprites/mysprite.h

Converts a PNG image to a C header with RGB565 pixel data stored in PROGMEM.
Supports transparency: pixels with alpha < 128 are written as 0x0000 (black).
"""

import argparse
import struct
import sys

try:
    from PIL import Image
except ImportError:
    print("Install Pillow: pip install Pillow", file=sys.stderr)
    sys.exit(1)


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    """Convert 8-bit RGB to 16-bit RGB565."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def convert(input_path: str, name: str, output_path: str) -> None:
    img = Image.open(input_path).convert("RGBA")
    w, h = img.size
    pixels = list(img.getdata())

    with open(output_path, "w") as f:
        f.write(f"// Auto-generated from {input_path}\n")
        f.write(f"// {w}x{h} RGB565\n")
        f.write(f"#pragma once\n")
        f.write(f"#include <stdint.h>\n\n")
        f.write(f"#ifndef PROGMEM\n#define PROGMEM\n#endif\n\n")
        f.write(f"const uint16_t {name}[{w * h}] PROGMEM = {{\n")

        for i, (r, g, b, a) in enumerate(pixels):
            if a < 128:
                val = 0x0000
            else:
                val = rgb888_to_rgb565(r, g, b)

            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{val:04X}")
            if i < len(pixels) - 1:
                f.write(", ")
            if (i + 1) % 12 == 0:
                f.write("\n")

        f.write(f"\n}};\n\n")
        f.write(f"const int {name}_W = {w};\n")
        f.write(f"const int {name}_H = {h};\n")

    print(f"Converted {input_path} -> {output_path} ({w}x{h}, {w*h*2} bytes)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="PNG to RGB565 PROGMEM converter")
    parser.add_argument("input", help="Input PNG file")
    parser.add_argument("--name", required=True, help="C variable name for the array")
    parser.add_argument("--out", required=True, help="Output .h file path")
    args = parser.parse_args()

    convert(args.input, args.name, args.out)
