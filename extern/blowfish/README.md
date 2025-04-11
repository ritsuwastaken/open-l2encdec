# [Blowfish](https://github.com/ddokkaebi/Blowfish)

Blowfish C++ implementation

[![image](http://i.creativecommons.org/p/zero/1.0/88x31.png)](http://creativecommons.org/publicdomain/zero/1.0/)

- Tested on OSX (32bit/64bit)
- ECB mode only
- The key length must be a multiple of 8bit

## Usage

```cpp
#include <iostream>
#include "blowfish.h"

int main(int argc, const char * argv[])
{
    unsigned char key[] = "The quick brown fox jumps over the lazy dog.";

    Blowfish blowfish;
    blowfish.SetKey(key, sizeof(key));

    // Input/Output length must be a multiple of the block length (64bit)
    unsigned char text[16] = "There's nothing";

    blowfish.Encrypt(text, text, sizeof(text));
    std::cout << text << std::endl;

    blowfish.Decrypt(text, text, sizeof(text));
    std::cout << text << std::endl;

    return 0;
}
```
