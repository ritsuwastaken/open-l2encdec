# examples/txt211json

A tool for merging Lineage II C1 item and skill data with names and descriptions, then exporting the result to JSON.

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

## Usage

```shell
$ ./txt211json <main_file> <meta_file> [output_file]
```

- `main_file`: Path to main file, e.g. armorgrp.txt, skillgrp.txt
- `meta_file`: Path to meta file: itemname-{}.txt, skillname-{}.txt
- `output_path`: (Optional) Path where json file will be saved (default: "output.json")

## Build

```shell
$ mkdir -p build && cd build
$ cmake .. && make
```

## Credits

- **nemtrif** - [utfcpp](https://github.com/nemtrif/utfcpp)
- **nlohmann** - [json](https://github.com/nlohmann/json)
