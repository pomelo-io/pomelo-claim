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

function approve(grant_id: string): AnyAction {
  return {
      account: "claim.pomelo",
      name: "approve",
      authorization: [permissionLevel],
      data: {
          round_id,
          grant_id,
          approved: true
      }
  };
};

(async () => {
  const actions: AnyAction[] = [];
  for ( const [grant_it] of JSON.parse(fs.readFileSync(filename, "utf8")) ) {
    actions.push(approve(grant_it));
  }
  await transact(actions);
})();