# open-l2encdec

A tool for encrypting and decrypting Lineage 2 game files across multiple protocols and platforms.  
Based on **l2encdec** by **DStuff** and **L2crypt** by **acmi**.

#### Supported protocol headers (Lineage2Ver###)

- **XOR**: 111, 120, 121
- **Blowfish**: 211, 212
- **RSA**: 411, 412, 413, 414 - **l2encdec** key by default

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

### [CLI usage and build options](/cli)

### [API](/API.md)

## Usage

In `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.14)
project(example VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
    l2encdec
    GIT_REPOSITORY https://github.com/ritsuwastaken/open-l2encdec.git
    GIT_TAG 1.3.9
)
fetchcontent_makeavailable(l2encdec)

add_library(${PROJECT_NAME} main.cpp) # or add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
```

In `main.cpp`

```cpp
#include <l2encdec.h>
// ...
std::vector<unsigned char> input;
std::string filename = "l2_skilltime";
int protocol_version = 121;
bool use_legacy_decrypt_rsa = false;
std::vector<unsigned char> output;

// basic usage
l2encdec::decode(input, output, protocol_version, filename, use_legacy_decrypt_rsa);
// ...
l2encdec::encode(input, output, protocol_version, filename, use_legacy_decrypt_rsa);

// advanced usage
l2encdec::Params params{};
l2encdec::init_params(&params, protocol_version, filename, use_legacy_decrypt_rsa);
params.tail = "000000000000000000000000deadbeef00000000";
l2encdec::decode(input, output, params);
// ...
l2encdec::encode(input, output, params);
```

See [`txt211json`](https://github.com/ritsuwastaken/txt211json), [`utx121webp`](https://github.com/ritsuwastaken/utx121webp) or [`cli`](./cli) for more examples

## Bindings
- [javascript/typescript](/modules/typescript/)

## Known issues

- Metadata is missing in `111`, `120` and `121` tails when encrypting - original `l2encdec` bug
- Protocol `121` encryption and decryption require the original filename - specify the filename via options

## Credits

- **DStuff** - [l2encdec](https://web.archive.org/web/20111021065705/http://dstuff.luftbrandzlung.org/l2.php)
- [Hint](https://github.com/Hint-ru) - [L2 file decoder](https://web.archive.org/web/20241105235133/https://forum.zone-game.info/showthread.php?tid=16178)
- **acmi** - [L2crypt](https://github.com/acmi/L2crypt)
- **richgel999, Tenacious Software LLC, RAD Game Tools, Valve Software** - [miniz](https://github.com/richgel999/miniz)
- **Trusted Firmware** - [MbedTLS](https://github.com/Mbed-TLS/mbedtls)
- **avinal** - [blowfish](https://github.com/avinal/blowfish)
- **peterspackman** - [mingw-w64 toolchain](https://gist.github.com/peterspackman/8cf73f7f12ba270aa8192d6911972fe8) for CMake

## License

This project is licensed under MIT
