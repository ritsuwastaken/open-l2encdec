# open-l2encdec TypeScript bindings

## Installation

```bash
npm install open-l2encdec
```

## Usage

```typescript
import { createL2EncDec } from 'open-l2encdec';

const codec = await createL2EncDec();
const params = codec.initParams(413);
const input = new Uint8Array([1, 2, 3, 4, 5]);

const encoded = codec.encode(input, params);
codec.verifyChecksum(encoded);
const decoded = codec.decode(encoded, params);
```

## API

- `createL2EncDec(): Promise<L2EncDec>` creates an independent codec instance.
- `codec.initParams(protocol, filename = '', useLegacyDecryptRsa = false): Params`
  returns default parameters. Protocol 121 requires the filename. Legacy RSA
  parameters for protocols 411–414 support decryption only.
- `codec.encode(input, params): Uint8Array` encrypts the input.
- `codec.decode(input, params): Uint8Array` decrypts the input.
- `codec.verifyChecksum(input): void` verifies an encoded file's checksum and throws on mismatch.

## CLI

```bash
npx open-l2encdec -c encode -p 413 input.bin output.bin
npx open-l2encdec -c decode -p 413 output.bin input.bin
```

Options: `-c` / `--command`, `-p` / `--protocol`, `-f` / `--filename` for protocol 121,
and `-l` / `--legacy` for original RSA decryption keys.
