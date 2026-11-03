import createModule, { type Params } from "./l2encdec.js";

export type { Params, Type, EncodeResult, DecodeResult, ChecksumResult } from "./l2encdec.js";
export type ByteInput = Uint8Array | number[];
export type L2EncDec = Awaited<ReturnType<typeof createL2EncDec>>;

export async function createL2EncDec() {
  const module = await createModule();
  return {
    Type: module.Type,
    EncodeResult: module.EncodeResult,
    DecodeResult: module.DecodeResult,
    ChecksumResult: module.ChecksumResult,

    initParams(protocol: number, filename = "", useLegacyDecryptRsa = false): Params {
      return module.init_params(protocol, filename, useLegacyDecryptRsa);
    },

    encode(input: ByteInput, params: Params): Uint8Array {
      return module.encode(input, params);
    },

    decode(input: ByteInput, params: Params): Uint8Array {
      return module.decode(input, params);
    },

    verifyChecksum(input: ByteInput): void {
      module.verify_checksum(input);
    }
  };
}
