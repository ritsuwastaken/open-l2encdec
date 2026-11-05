# open-l2encdec

#### Supported protocol headers (Lineage2Ver###)

- **XOR**: 111, 120, 121*
- **Blowfish**: 211, 212
- **RSA**: 411, 412, 413, 414 - **l2encdec** key by default

\* Protocol 121 requires the filename.

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

## Usage

```cmake
cmake_minimum_required(VERSION 3.28)
project(example LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
    l2encdec
    GIT_REPOSITORY https://github.com/ritsuwastaken/open-l2encdec.git
    GIT_TAG 1.5.0
)
fetchcontent_makeavailable(l2encdec)

add_library(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
```

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
l2encdec::init_params(params, protocol_version, filename, use_legacy_decrypt_rsa);
params.tail = "000000000000000000000000deadbeef00000000";
l2encdec::decode(input, output, params);
// ...
l2encdec::encode(input, output, params);
```

See [`include/l2encdec.h`](./include/l2encdec.h) for the public API.  
See [`cli`](./cli) for extended usage examples.

## Credits

- **DStuff** - [l2encdec](https://web.archive.org/web/20111021065705/http://dstuff.luftbrandzlung.org/l2.php)
- [Hint](https://github.com/Hint-ru) - [L2 file decoder](https://web.archive.org/web/20241105235133/https://forum.zone-game.info/showthread.php?tid=16178)
- **acmi** - [L2crypt](https://github.com/acmi/L2crypt)
- **LibTom Projects** - [LibTomMath](https://github.com/libtom/libtommath) and [LibTomCrypt](https://github.com/libtom/libtomcrypt)
- **zlib-ng** - [zlib-ng](https://github.com/zlib-ng/zlib-ng)
