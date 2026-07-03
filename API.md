# API

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
    bool skip_header;
    std::string filename;
    int xor_key;
    int xor_start_position;
    std::string blowfish_key;
    std::string rsa_modulus;
    std::string rsa_public_exponent;
    std::string rsa_private_exponent;
};
```

Configuration for encryption/decryption.

---

```cpp
bool init_params(Params &params, int protocol, std::string filename = "", bool use_legacy_decrypt_rsa = false);
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
EncodeResult encode(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, int protocol, const std::string& filename);
```

Encode (compress/encrypt) input data using the specified protocol.

---

```cpp
DecodeResult decode(const std::vector<unsigned char>& input_data, std::vector<unsigned char>& output_data, const Params& params);
```

Decode (decrypt/decompress) input data using the specified parameters.

---

```cpp
DecodeResult decode(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, int protocol, const std::string& filename = "", bool use_legacy_rsa);
```

Decode (decrypt/decompress) input data using the specified protocol.
