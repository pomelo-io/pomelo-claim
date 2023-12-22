import { AnyAction, Session } from "@wharfkit/session";
import { WalletPluginPrivateKey } from "@wharfkit/wallet-plugin-privatekey";

if ( !process.env.PRIVATE_KEY) throw new Error("PRIVATE_KEY is required in .env");
const walletPlugin = new WalletPluginPrivateKey(process.env.PRIVATE_KEY)
export const permissionLevel = {actor: "claim.pomelo", permission: "setclaim"}

export const session = new Session({
    chain: { id: "aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906", url: "https://eos.greymass.com" },
    permissionLevel,
    walletPlugin,
});

export async function transact(actions: AnyAction[]) {
    try {
        const {response} = await session.transact({actions});
        if ( response ) {
          console.log("OK", response.transaction_id)
        } else {
          console.log("ERROR", response)
        }
      } catch (error) {
        console.error("ERROR", error)
      }
}