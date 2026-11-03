#!/usr/bin/env node

import fs from "node:fs/promises";
import { parseArgs } from "node:util";
import { createL2EncDec } from "./index.js";

async function main() {
  const { values, positionals } = parseArgs({
    options: {
      protocol: {
        type: "string",
        short: "p",
        default: "413"
      },
      command: {
        type: "string",
        short: "c",
        default: "decode"
      },
      filename: {
        type: "string",
        short: "f",
        default: "",
      },
      legacy: {
        type: "boolean",
        short: "l",
        default: false,
      }
    },
    allowPositionals: true
  });

  if (!['decode', 'encode'].includes(values.command)
    || !positionals[0]
    || !positionals[1]) {
    console.error(`
Usage:
  npx open-l2encdec -c encode -p 413 <input> <output>
  npx open-l2encdec -c decode -p 413 <input> <output>
`);
    process.exit(1);
  }

  const [input, codec] = await Promise.all([
    fs.readFile(positionals[0]),
    createL2EncDec()
  ]);
  const params = codec.initParams(parseInt(values.protocol), values.filename, values.legacy);
  const result = values.command === "encode"
    ? codec.encode(input, params)
    : codec.decode(input, params);

  await fs.writeFile(positionals[1], result);
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
