# examples/cli

l2encdec-like CLI

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

## Usage

Drag and drop file(s) onto the executable or use command line options

#### Options

- -h - prints help message
- -c _string_ - command - `encode` or `decode`. Defaults to `decode`
- -p _number_ - protocol - `111`, `120`, `121`, `211`, `212`, `411`, `412`, `413`, `414`
- -o _string_ - output file path
- -v - verify checksum in the tail before decoding (the game client doesn't do it)
- -t - do not add tail/read file without tail (e.g., for Exteel files)
- -f _string_ - force different filename for `xor_filename` - protocol `121`
- -l - use legacy RSA credentials for decryption; only for protocols `411-414`

<details>
<summary>Advanced options</summary>

- -a _string_ - encryption algorithm - `blowfish`, `rsa`, `xor`, `xor_position`, `xor_filename`
- -m _string_ - custom modulus for `rsa` in hex
- -e/-d _string_ - custom public or private exponent for `rsa` in hex
- -b _string_ - custom key for `blowfish`
- -x _string_ - custom key for `xor` in hex
- -s _string_ - custom start index for `xor_position` in hex
- -w _string_ - custom wide char header; default: Lineage2Ver + protocol
- -T _string_ - custom tail for encoding, must be exactly 40 characters (20 bytes), e.g., `000000000000000000000000deadbeef00000000`; contains checksum by default
</details>

#### Examples

```shell
# Decode a file
$ ./l2encdec -c decode filename.ini
# Encode a file using protocol 413
$ ./l2encdec -c encode -p 413 -o enc-filename.ini dec-filename.ini
# Decode a file with custom RSA modulus and exponent
$ ./l2encdec -c decode -a rsa -m 75b4d6...e2039 -d 1d -w Lineage2Ver413 -o dec-filename.ini filename.ini
```

## Build

#### CMake

- [CMake](https://cmake.org/download/) >= 3.14
- [Git](https://git-scm.com/downloads)

```shell
$ cd examples/cli
$ cmake . --preset unix
$ cmake --build --preset unix-build
```

#### Docker / Podman

|                                                    | [Docker](https://docs.docker.com/get-started/get-docker/) >= 27.2.0                                                                               | [Podman](https://podman.io/docs/installation) >= 1.17.2                                                                                           |
| -------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| Build container image                              | `docker build -f examples/cli/docker/win64.Dockerfile -t l2encdec-builder .`                                                                      | `podman build -f examples/cli/docker/win64.Dockerfile -t l2encdec-builder .`                                                                      |
| Either extract executable to host                  | `mkdir -p build && docker run --rm -v "$(pwd)/build:/mounted-build" l2encdec-builder`                                                             | `mkdir -p build && podman run --rm -v "$(pwd)/build:/mounted-build" l2encdec-builder`                                                             |
| Or run tool directly on files in current directory | `docker run --rm -v "$(pwd):/app/data" --entrypoint "/build/l2encdec.exe" l2encdec-builder -c decode -o /app/data/output.dat /app/data/input.dat` | `podman run --rm -v "$(pwd):/app/data" --entrypoint "/build/l2encdec.exe" l2encdec-builder -c decode -o /app/data/output.dat /app/data/input.dat` |

#### Visual Studio (as CMake project)

- [Visual Studio 17 2022 or newer](https://visualstudio.microsoft.com/downloads/)
- [CMake](https://cmake.org/) - [Included with Visual Studio](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#installation)

## Credits

- **Microsoft** - [getopt](https://github.com/iotivity/iotivity/blob/master/resource/c_common/windows/src/getopt.c) from [IoTivity](https://github.com/iotivity/iotivity) project
