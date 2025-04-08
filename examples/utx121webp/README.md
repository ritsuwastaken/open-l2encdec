# examples/utx121webp

Only for extracting DXT1 and DXT3 textures from Lineage II UTX files.  
This tool is based on [UE Viewer](https://www.gildor.org/en/projects/umodel).

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

## Usage

```shell
./utx121webp <input_file> [output_path]
```

- `input_file`: Path to the encrypted UTX file
- `output_path`: (Optional) Path where .webp files will be saved (default: `webp_output/{package_name}`)

## Build

```shell
mkdir -p build && cd build
cmake .. && make
```

## Credits

- **Glidor** - [UE Viewer](https://github.com/gildor2/UEViewer)
- **Google** - [libwebp](https://github.com/webmproject/libwebp)
