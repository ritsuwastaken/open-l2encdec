# l2encdec/examples

See [`txt211json`](./txt211json), [`utx121webp`](./utx121webp) or [`tools/cli`](../tools/cli) for more usage examples

## Usage

In `CMakeLists.txt`

```cmake
include(FetchContent)

FetchContent_Declare(
    l2encdec
    GIT_REPOSITORY https://github.com/ritsuwastaken/open-l2encdec.git
    GIT_TAG 1.1.9
)
FetchContent_MakeAvailable(l2encdec)

target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
```

In your code

```cpp
#include <l2encdec.h>
// ...
std::vector<unsigned char> input;
std::string filename = "";
int protocol_version = 121;

l2encdec::Params params;
l2encdec::init_params(&params, protocol_version, filename);
std::vector<unsigned char> output;
l2encdec::decode(input, output, params);
// or
l2encdec::encode(input, output, params);
```

## API

Configuration for encryption/decryption:

```cpp
enum class Type
{
    NONE,
    XOR,
    XOR_FILENAME,
    XOR_POSITION,
    BLOWFISH,
    RSA
};

struct Params {
    Type type;
    std::string header;
    std::string tail;
    bool skip_tail;
    std::string filename;
    int xor_key;
    int xor_start_position;
    std::string blowfish_key;
    std::string rsa_modulus;
    std::string rsa_public_exponent;
    std::string rsa_private_exponent;
};
```

---

```cpp
bool init_params(Params* params, int protocol, std::string filename = "", bool use_legacy_decrypt_rsa = false);
```

Initialize default parameters for the specified protocol.

- `params`: Pointer to the Params struct to fill
- `protocol`: Last 3 digits of the file header
- `filename`: Used to compute XOR key (for protocol 121)
- `use_legacy_decrypt_rsa`: Use legacy keys for 411–414
- Returns `true` if successful

---

```cpp
ChecksumResult verify_checksum(const std::vector<unsigned char>& input_data);
```

Verify checksum of the encrypted input data.

---

```cpp
EncodeResult encode(const std::vector<unsigned char>& input_data, std::vector<unsigned char>& output_data, const Params& params);
```

Encode (compress/encrypt) input data using the specified parameters.

---

```cpp
DecodeResult decode(const std::vector<unsigned char>& input_data, std::vector<unsigned char>& output_data, const Params& params);
```

Decode (decrypt/decompress) input data using the specified parameters.
