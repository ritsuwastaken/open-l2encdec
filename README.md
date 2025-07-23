# open-l2encdec

A tool for encrypting and decrypting Lineage 2 game files across multiple protocols and platforms.  
Available both as a standalone executable and [as a library](/examples).  
Based on **l2encdec** by **DStuff** and **L2crypt** by **acmi**.

#### Supported protocol headers (Lineage2Ver###)

- **XOR**: 111, 120, 121
- **Blowfish**: 211, 212
- **RSA**: 411, 412, 413, 414 - **l2encdec** key by default

### [Download](https://github.com/ritsuwastaken/open-l2encdec/releases/latest)

### [CLI usage and build options](/tools/cli)

## Known issues

- Metadata is missing in `111`, `120` and `121` tails when encrypting - original `l2encdec` bug
- Protocol `121` encryption and decryption require the original filename - specify the name via options if the file was renamed

## Credits

- **DStuff** - [l2encdec](https://web.archive.org/web/20111021065705/http://dstuff.luftbrandzlung.org/l2.php)
- **Hint** - [L2 file decoder](https://web.archive.org/web/20241105235133/https://forum.zone-game.info/showthread.php?tid=16178)
- **acmi** - [L2crypt](https://github.com/acmi/L2crypt)
- **richgel999, Tenacious Software LLC, RAD Game Tools, Valve Software** - [miniz](https://github.com/richgel999/miniz)
- **Trusted Firmware** - [MbedTLS](https://github.com/Mbed-TLS/mbedtls)
- **ddokkaebi, picoHz** - [Blowfish C++ implementation](https://github.com/ddokkaebi/Blowfish)
- **peterspackman** - [mingw-w64 toolchain](https://gist.github.com/peterspackman/8cf73f7f12ba270aa8192d6911972fe8) for CMake

## License

This project is licensed under MIT
