#!/usr/bin/env python3
"""Convert a PNG image to an LVGL 9 C array (RGB565A8 format).

Usage:
    python3 png_to_lvgl.py input.png output.c --name img_haz1 --size 64

Generates a .c file with a const lv_image_dsc_t that can be used directly
with lv_image_set_src().
"""

import argparse
import struct
from pathlib import Path
from PIL import Image


def rgb888_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def convert(input_path, output_path, var_name, size):
    img = Image.open(input_path).convert("RGBA")
    img = img.resize((size, size), Image.LANCZOS)

    width, height = img.size
    pixels = list(img.getdata())

    # Build RGB565 data and alpha data separately
    rgb_data = bytearray()
    alpha_data = bytearray()

    for r, g, b, a in pixels:
        rgb565 = rgb888_to_rgb565(r, g, b)
        rgb_data += struct.pack("<H", rgb565)  # little-endian uint16
        alpha_data.append(a)

    full_data = rgb_data + alpha_data
    stride = width * 2  # bytes per row for RGB565

    # Write C file
    with open(output_path, "w") as f:
        f.write(f"/* Auto-generated from {Path(input_path).name} — do not edit */\n")
        f.write('#include "lvgl.h"\n\n')

        # Data array
        f.write(f"static const uint8_t {var_name}_data[] = {{\n")
        for i, byte in enumerate(full_data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x},")
            if i % 16 == 15 or i == len(full_data) - 1:
                f.write("\n")
            else:
                f.write(" ")
        f.write("};\n\n")

        # Image descriptor
        f.write(f"const lv_image_dsc_t {var_name} = {{\n")
        f.write(f"    .header = {{\n")
        f.write(f"        .magic = LV_IMAGE_HEADER_MAGIC,\n")
        f.write(f"        .cf = LV_COLOR_FORMAT_RGB565A8,\n")
        f.write(f"        .flags = 0,\n")
        f.write(f"        .w = {width},\n")
        f.write(f"        .h = {height},\n")
        f.write(f"        .stride = {stride},\n")
        f.write(f"        .reserved_2 = 0,\n")
        f.write(f"    }},\n")
        f.write(f"    .data_size = sizeof({var_name}_data),\n")
        f.write(f"    .data = {var_name}_data,\n")
        f.write(f"}};\n")

    print(f"Converted: {input_path} -> {output_path}")
    print(f"  {var_name}: {width}x{height}, RGB565A8, {len(full_data)} bytes")


def main():
    parser = argparse.ArgumentParser(description="Convert PNG to LVGL 9 C array")
    parser.add_argument("input", help="Input PNG file")
    parser.add_argument("output", help="Output C file")
    parser.add_argument("--name", required=True, help="C variable name")
    parser.add_argument("--size", type=int, default=64, help="Resize to NxN (default 64)")
    args = parser.parse_args()
    convert(args.input, args.output, args.name, args.size)


if __name__ == "__main__":
    main()
