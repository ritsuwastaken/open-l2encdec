# open-l2encdec typescript bindings

## Installation

```bash
$ npm install open-l2encdec
# or
$ yarn add open-l2encdec
# or
$ pnpm add open-l2encdec
# or
$ bun add open-l2encdec
```

## Usage

```typescript
// node
import { initL2EncDec, encode, decode, initParams } from 'open-l2encdec';
// browser
import { initL2EncDec, encode, decode, initParams } from 'open-l2encdec/web';

const module = await initL2EncDec();

const params = await initParams(111);
const input = new Uint8Array([1, 2, 3, 4, 5]);
const encoded = await encode(input, params, module);

const decoded = await decode(encoded, params, module);
```

## API Reference

#### `initL2EncDec(): Promise<MainModule>`

Initializes and returns the l2encdec WASM module. The module is cached, so subsequent calls return the same instance.

#### `encode(input: Uint8Array | number[], params: Params, module?: MainModule): Promise<Uint8Array>`

Encodes input data using the specified parameters.

- `input`: Input data as `Uint8Array` or array of numbers
- `params`: Encoding parameters (see `initParams` or `Params` type)
- `module`: Optional pre-initialized module (for performance)

Returns encoded data as `Uint8Array`.

#### `decode(input: Uint8Array | number[], params: Params, module?: MainModule): Promise<Uint8Array>`

Decodes input data using the specified parameters.

- `input`: Input data as `Uint8Array` or array of numbers
- `params`: Decoding parameters (see `initParams` or `Params` type)
- `module`: Optional pre-initialized module (for performance)

Returns decoded data as `Uint8Array`.

#### `initParams(protocol: number, filename?: string, use_legacy_decrypt_rsa?: boolean, module?: MainModule): Promise<Params>`

Initializes default parameters for the specified protocol.

- `protocol`: Protocol number (last three digits of file header)
- `filename`: Filename used for protocol 121 (XOR_FILENAME)
- `use_legacy_decrypt_rsa`: Use legacy (original) RSA decryption for protocols 411-414
- `module`: Optional pre-initialized module (for performance)

## Development

- [CMake](https://cmake.org/) >= 3.14
- [Emscripten](https://emscripten.org/) >= 4.0.21
- [Node.js](https://nodejs.org/) >= v22
- [npm](https://www.npmjs.com/), [yarn](https://yarnpkg.com/), [pnpm](https://pnpm.io/) or [bun](https://bun.sh/)

### Building

```bash
$ npm run build
# or
$ yarn build
# or
$ pnpm build
# or
$ bun run build
```

## License

MIT License - see [LICENSE](../../LICENSE) file for details.
