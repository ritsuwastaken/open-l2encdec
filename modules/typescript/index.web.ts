import type {
  MainModule,
  Params,
  Type,
  EncodeResult,
  DecodeResult
} from "./l2encdec.js";

export type {
  MainModule as L2EncDecModule,
  Params,
  Type,
  EncodeResult,
  DecodeResult
};

let cachedModule: MainModule | undefined;

async function loadModule(): Promise<MainModule> {
  if (cachedModule) return cachedModule;

  // @ts-ignore - generated file
  const mod = await import("./l2encdec.browser.js");
  cachedModule = (await mod.default()) as MainModule;
  return cachedModule;
}

export async function initL2EncDec(): Promise<MainModule> {
  return loadModule();
}

export async function encode(
  input: Uint8Array | number[],
  params: Params,
  module?: MainModule
): Promise<Uint8Array> {
  const m = module ?? await loadModule();
  const result = m.encode(input, params);
  return result instanceof Uint8Array ? result : new Uint8Array(result);
}

export async function decode(
  input: Uint8Array | number[],
  params: Params,
  module?: MainModule
): Promise<Uint8Array> {
  const m = module ?? await loadModule();
  const result = m.decode(input, params);
  return result instanceof Uint8Array ? result : new Uint8Array(result);
}

export async function initParams(
  protocol: number,
  filename?: string,
  use_legacy_decrypt_rsa?: boolean,
  module?: MainModule
): Promise<Params> {
  const m = module ?? await loadModule();
  const params = new m.Params();
  m.init_params(
    params,
    protocol,
    filename ?? "",
    use_legacy_decrypt_rsa ?? false
  );
  return params;
}
