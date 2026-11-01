import createModule from "./l2encdec.browser.js";
import { createCodec } from "./api.js";

export type { L2EncDec, ByteInput, Params, Type, EncodeResult, DecodeResult, ChecksumResult } from "./api.js";

export function createL2EncDec() {
  return createCodec(createModule);
}
