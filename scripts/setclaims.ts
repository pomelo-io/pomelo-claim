import { Session, AnyAction, Asset } from "@wharfkit/session"
import { WalletPluginPrivateKey } from "@wharfkit/wallet-plugin-privatekey"
import fs from 'fs';
import path from "path";

const filename = process.argv[2];
const privateKey = process.argv[3];

if (!filename || !privateKey) {
  console.log("Usage: node setclaims.js <filename> <privateKey>")
  process.exit(1)
}
if (!fs.existsSync(filename)) {
  console.log("File not found: " + filename)
  process.exit(1)
}

const round_id = path.parse(filename).name;
const walletPlugin = new WalletPluginPrivateKey(privateKey)
const permissionLevel = {actor: "claim.pomelo", permission: "setclaim"}

function setclaim(grant_id: string, claim: string ): AnyAction {
  const symbol = Asset.from(claim).symbol.toString();
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

const session = new Session({
  chain: { id: "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906", url: "https://eos.greymass.com" },
  permissionLevel,
  walletPlugin,
});

(async () => {
  const actions: AnyAction[] = [];
  for ( const [grant_it, quantity] of JSON.parse(fs.readFileSync(filename, "utf8")) ) {
    actions.push(setclaim(grant_it, quantity));
  }
  try {
    await session.transact({actions});
    console.log("OK")
  } catch (error) {
    console.error("ERROR", error)
  }
})();