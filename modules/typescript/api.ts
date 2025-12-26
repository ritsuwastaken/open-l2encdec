import type {
  MainModule,
  Params,
  Type,
  EncodeResult,
  DecodeResult
} from "./l2encdec.node.js";

export type {
  MainModule as L2EncDecModule,
  Params,
  Type,
  EncodeResult,
  DecodeResult
};

export function createApi(createModule: () => Promise<MainModule>) {
  let modulePromise: Promise<MainModule> | undefined;

  function initL2EncDec(): Promise<MainModule> {
    modulePromise ??= createModule().catch(error => {
      modulePromise = undefined;
      throw error;
    });
    return modulePromise;
  }

  async function encode(
    input: Uint8Array | number[],
    params: Params,
    module?: MainModule
  ): Promise<Uint8Array> {
    const m = module ?? await initL2EncDec();
    return m.encode(input, params);
  }

  async function decode(
    input: Uint8Array | number[],
    params: Params,
    module?: MainModule
  ): Promise<Uint8Array> {
    const m = module ?? await initL2EncDec();
    return m.decode(input, params);
  }

  async function initParams(
    protocol: number,
    filename = "",
    use_legacy_decrypt_rsa = false,
    module?: MainModule
  ): Promise<Params> {
    const m = module ?? await initL2EncDec();
    const params = new m.Params();
    try {
      m.init_params(params, protocol, filename, use_legacy_decrypt_rsa);
      return params;
    } catch (error) {
      params.delete();
      throw error;
    }
  }

  return { initL2EncDec, encode, decode, initParams };
}
