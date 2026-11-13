import createModule, { type Params } from "./l2encdec.js";

export type { Params, Type, EncodeResult, DecodeResult, ChecksumResult } from "./l2encdec.js";
export type ByteInput = Uint8Array | number[];
export type L2EncDec = Awaited<ReturnType<typeof createL2EncDec>>;

type NativeResult = { code: number; data?: Uint8Array };

const ENCODE_ERRORS: Record<number, string> = {
  [-1]: "Invalid encryption type",
  [-2]: "Compression failed",
  [-3]: "Encryption failed",
  [-4]: "Cancelled",
};

const DECODE_ERRORS: Record<number, string> = {
  [-1]: "Invalid encryption type",
  [-2]: "Decompression failed",
  [-3]: "Decryption failed",
  [-4]: "Cancelled",
};

function error(name: string, code: number, message: string): Error {
  const err = new Error(message) as Error & { code: number };
  err.name = name;
  err.code = code;
  return err;
}

function unwrap(result: NativeResult, name: string, messages: Record<number, string>): Uint8Array {
  if (result.code !== 0 || !result.data)
    throw error(name, result.code, messages[result.code] ?? `Unknown ${name === "EncodeError" ? "encode" : "decode"} error`);
  return result.data;
}

export async function createL2EncDec() {
  const module = await createModule();
  return {
    Type: module.Type,
    EncodeResult: module.EncodeResult,
    DecodeResult: module.DecodeResult,
    ChecksumResult: module.ChecksumResult,

    initParams(protocol: number, filename = "", useLegacyDecryptRsa = false): Params {
      const params = module.init_params(protocol, filename, useLegacyDecryptRsa) as Params | null;
      if (!params)
        throw error("InitParamsError", -1, "Failed to initialize parameters");
      return params;
    },

    encode(input: ByteInput, params: Params): Uint8Array {
      return unwrap(module.encode(input, params) as NativeResult, "EncodeError", ENCODE_ERRORS);
    },

    decode(input: ByteInput, params: Params): Uint8Array {
      return unwrap(module.decode(input, params) as NativeResult, "DecodeError", DECODE_ERRORS);
    },

    verifyChecksum(input: ByteInput): void {
      const code = module.verify_checksum(input) as number;
      if (code !== 0)
        throw error("ChecksumError", code, "Checksum mismatch");
    }
  };
}
