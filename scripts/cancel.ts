import { AnyAction } from "@wharfkit/session"
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

function cancel(grant_id: string): AnyAction {
  return {
      account: "claim.pomelo",
      name: "cancel",
      authorization: [permissionLevel],
      data: {
          round_id,
          grant_id
      }
  };
};

(async () => {
  const actions = [];
  for ( const [grant_it] of JSON.parse(fs.readFileSync(filename, "utf8")) ) {
    actions.push(cancel(grant_it));
  }
  await transact(actions);
})();