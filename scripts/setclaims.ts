import { AnyAction, Asset } from "@wharfkit/session"
import fs from 'fs';
import path from "path";
import { permissionLevel, transact } from "./config";

const filename = process.argv[2];

if (!filename) {
  console.log("Usage: node setclaims.js <filename>")
  process.exit(1)
}
if (!fs.existsSync(filename)) {
  console.log("File not found: " + filename)
  process.exit(1)
}

const round_id = path.parse(filename).name;

function setclaim(grant_id: string, claim: string ): AnyAction {
  const symbol = Asset.from(claim).symbol.code.toString();
  const contract = symbol === "TLOS" ? "ibc.wt.tlos" : "eosio.token";
  return {
      account: "claim.pomelo",
      name: "setclaim",
      authorization: [permissionLevel],
      data: {
          round_id,
          grant_id,
          claim: {contract, quantity: claim}
      }
  };
};

(async () => {
  const actions: AnyAction[] = [];
  for ( const [grant_it, quantity] of JSON.parse(fs.readFileSync(filename, "utf8")) ) {
    actions.push(setclaim(grant_it, quantity));
  }
  await transact(actions);
})();