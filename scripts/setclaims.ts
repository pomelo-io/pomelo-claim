import { Session, WalletPluginPrivateKey, AnyAction } from "@wharfkit/session"
import fs from 'fs';

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

const walletPlugin = new WalletPluginPrivateKey({privateKey})
const permissionLevel = {actor: "claim.pomelo", permission: "setclaim"}

function setclaim(grant_id: string, claim: string ): AnyAction {
  return {
      account: "claim.pomelo",
      name: "setclaim",
      authorization: [permissionLevel],
      data: {
          round_id: 401,
          grant_id,
          claim: {contract: "eosio.token", quantity: claim}
      }
  };
};

const session = new Session({
  chain: { id: "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906", url: "https://eos.greymass.com" },
  permissionLevel,
  walletPlugin,
});

(async () => {
  for ( const [grant_it, quantity] of JSON.parse(fs.readFileSync(filename, "utf8")) ) {
    try {
      const action = setclaim(grant_it, quantity);
      await session.transact({action});
      console.log("OK", grant_it, quantity)
    } catch (e) {
      console.error("ERROR", grant_it, quantity)
    }
  }
})();