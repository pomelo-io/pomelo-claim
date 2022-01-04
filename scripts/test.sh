#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# enable tokens
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo
cleos push action app.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p app.pomelo

# create and link eosn acc
cleos push action login.eosn create '["prjman1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn link '["prjman1.eosn", "prjman1", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p prjman1
cleos push action login.eosn configsocial '["shufti", 5000]' -p login.eosn

# create matching round and start it
cleos push action app.pomelo setconfig '[0, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2022-05-25T20:00:00", "2021-05-19T20:00:00", "2022-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setgrant '["prjman1.eosn", "grant1", "prjman1", ["EOS", "USDT"]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn

cleos push action app.pomelo setgrant '["prjman1.eosn", "grant2", "prjman1", ["EOS", "USDT"]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo setstate '["grant2", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant2", 101]' -p app.pomelo -p prjman1.eosn

# set claim config
echo "Set config ✅"
cleos push action claim.pomelo setconfig '{"config":["ok", "login.eosn", "app.pomelo", "match.pomelo", ["shufti"], 180]}' -p claim.pomelo

# transfer funds from the vault
echo "Transfer funds from the vault ✅"
cleos transfer match.pomelo claim.pomelo "900.0000 EOS" "grant:grant1"
echo "Transfer funds from the vault ✅"
cleos transfer match.pomelo claim.pomelo "100.0000 EOS" "grant:grant1"
echo "Transfer funds from the vault ✅"
cleos transfer match.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether
echo "Transfer funds from the vault ✅"
cleos transfer match.pomelo claim.pomelo "100.0000 EOS" "grant:grant2"
echo "Transfer funds from the vault ✅"

# claim funds without KYC - must fail
echo "Claim funds without KYC - must fail ❌"
cleos push action claim.pomelo claim '[prjman1, grant1]' -p prjman1

# set KYC social
echo "Pass KYC ✅"
cleos push action login.eosn social '[prjman1.eosn, shufti]' -p prjman1.eosn

# claim funds with KYC
echo "Claim funds for grant2 ✅"
cleos push action claim.pomelo claim '[prjman1, grant1]' -p prjman1
echo "Claim funds for grant2 ✅"
cleos push action claim.pomelo claim '[prjman1, grant2]' -p prjman1

# sleep to avoid duplicate trx
sleep 1
echo "Transfer funds from the vault ✅"
# transfer funds from the vault
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "grant:grant1"
echo "Transfer funds from the vault ✅"
cleos transfer match.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether

# reclaim funds - must fail
echo "Reclaim funds with wrong authority - must fail ❌"
cleos push action claim.pomelo reclaim '[grant1]' -p prjman1

# reclaim funds
echo "Reclaim funds back to the vault ✅"
cleos push action claim.pomelo reclaim '[grant1]' -p match.pomelo