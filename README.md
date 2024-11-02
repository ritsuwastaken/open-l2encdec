# open-l2encdec
Based on **l2encdec** by **DStuff** and **L2crypt** by **acmi**. Multiplatform. Can be used as a library.

#### Supported methods and headers
- **XOR**: 111, 120, 121
- **Blowfish**: 211, 212
- **RSA**: 411, 412, 413, 414 - **l2encdec** key by default

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

## Usage
Drag and drop file(s) onto the executable or use command line options

#### Options
- -h - print help
- -c *string* - command - `encode` or `decode`. Defaults to `decode`
- -p *number* - protocol - 111, 120, 121, 211, 212, 411, 412, 413, 414
- -o *string* - output file path
- -t - skip tail verification for decoding; do not add tail when encoding
- -f *string* - force different filename for `xor_filename` - protocol 121
- -l - use legacy RSA credentials for decryption; only for protocols 411-414

#### Advanced options
- -a *string* - encryption algorithm - `blowfish`, `rsa`, `xor`, `xor_position`, `xor_filename`
- -m *string* - custom modulus for RSA in hex
- -e/-d *hex* - custom public or private exponent for RSA in hex
- -b *string* - custom key for Blowfish
- -x *string* - custom key for XOR in hex
- -s *string* - custom start index for XOR position in hex
- -w *string* - custom wide char header; default: Lineage2Ver + protocol

#### Examples
```bash
# Decode a file
$ ./l2encdec -c decode filename.ini
# Encode a file using protocol 413
$ ./l2encdec -c encode -p 413 -o enc-filename.ini dec-filename.ini
# Decode a file with custom RSA modulus and exponent
$ ./l2encdec -c decode -a rsa -m 75b4d6...e2039 -d 1d -w Lineage2Ver413 -o dec-filename.ini filename.ini
```

## Changes
- Doesn't include `loader.exe`, `loaderCT1.exe`, `gg-bps.dll` and `patcher.exe`
- Replaced `zlib` with `miniz`, added support for `mini-gmp`

## Known issues
- Missing metadata in 111, 120 and 121 tails - original `l2encdec` bug
- Protocol 121 encryption/decryption requires original filename, use `-f` option to force it
- Input/output paths with spaces are not supported

## Build
#### CMake
- [cmake](https://cmake.org/download/) >= 3.28
- [gmp](https://gmplib.org/) >= 6.2 - optional, but recommended for performance
```shell
$ cmake . -B 'build'
$ cmake --build 'build'
```
#### Docker
```shell
$ mkdir -p build
$ docker build -t l2encdec-builder .  
$ docker run --rm -v "$(pwd)/build:/mounted-build" l2encdec-builder
```
#### Visual Studio (as CMake project)
- [Visual Studio 17 2022 or newer](https://visualstudio.microsoft.com/downloads/)
- [vcpkg](https://github.com/microsoft/vcpkg) - [Included with Visual Studio](https://devblogs.microsoft.com/cppblog/vcpkg-is-now-included-with-visual-studio/)

## Credits
- **DStuff** - [l2encdec](https://web.archive.org/web/20111021065705/http://dstuff.luftbrandzlung.org/l2.php)
- **Hint** - [L2 file decoder](https://archive.ph/i7GPD)
- **acmi** - [L2crypt](https://github.com/acmi/L2crypt)
- **ddokkaebi** - [Blowfish](https://github.com/ddokkaebi/Blowfish)
- **richgel999** - [miniz](https://github.com/richgel999/miniz)
- **Free Software Foundation** - [gmp, mini-gmp](https://gmplib.org/)
- **Microsoft** - [getopt](https://github.com/iotivity/iotivity) from IoTivity project

## License
This project is licensed under MIT
