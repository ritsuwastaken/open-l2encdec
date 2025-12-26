import createModule from "./l2encdec.browser.js";
import { createApi } from "./api.js";

export type { L2EncDecModule, Params, Type, EncodeResult, DecodeResult } from "./api.js";
export const { initL2EncDec, encode, decode, initParams } = createApi(createModule);
