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
    GIT_TAG 2.0.0
)
fetchcontent_makeavailable(l2encdec)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
```

```cpp
#include <l2encdec.h>
#include <fstream>
#include <iterator>

int main()
{
    const auto filename = "l2_skilltime";
    std::ifstream file("l2_skilltime.utx", std::ios::binary);
    if (!file) return 1;
    const std::vector<unsigned char> input(std::istreambuf_iterator<char>{file}, {});
    std::vector<unsigned char> decoded, encoded;

    l2encdec::decode(input, decoded, 121, filename);
    // ...
    l2encdec::encode(decoded, encoded, 121, filename);

    // with custom params
    l2encdec::Params params{};
    l2encdec::init_params(params, 121, filename);
    params.tail = "000000000000000000000000deadbeef00000000";
    l2encdec::decode(input, decoded, params);
    // ...
    l2encdec::encode(decoded, encoded, params);
}
```

See [`include/l2encdec.h`](./include/l2encdec.h) for the public API.  
See [`cli`](./cli) for extended usage examples.

## Bindings

- [javascript/typescript](/modules/typescript/)

## Credits

- **DStuff** - [l2encdec](https://web.archive.org/web/20111021065705/http://dstuff.luftbrandzlung.org/l2.php)
- [Hint](https://github.com/Hint-ru) - [L2 file decoder](https://web.archive.org/web/20241105235133/https://forum.zone-game.info/showthread.php?tid=16178)
- **acmi** - [L2crypt](https://github.com/acmi/L2crypt)
- **LibTom Projects** - [LibTomMath](https://github.com/libtom/libtommath) and [LibTomCrypt](https://github.com/libtom/libtomcrypt)
- **zlib-ng** - [zlib-ng](https://github.com/zlib-ng/zlib-ng)
