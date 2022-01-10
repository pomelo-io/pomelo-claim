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

cleos push action login.eosn create '["user1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn link '["user1.eosn", "user1", "SIG_K1_JzJsKRsfVsDJv52xz1DQakvmXQ2NeJr1kwGYrtt4ttN6Cudzm4wG5fmS1ak7JCVRJabM7sYGwk5gpX8TBq2EymQQ1LNUe3"]' -p user1

# create matching round and start it
cleos push action app.pomelo setconfig '[0, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2022-05-25T20:00:00", "2021-05-19T20:00:00", "2022-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo
cleos push action app.pomelo setconfig '[1, 500, 500, null, null]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setgrant '["prjman1.eosn", "grant1", "prjman1", ["EOS", "USDT"]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn

cleos push action app.pomelo setgrant '["prjman1.eosn", "grant2", "prjman1", ["EOS", "USDT"]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo setstate '["grant2", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant2", 101]' -p app.pomelo -p prjman1.eosn

cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant1"
cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant2"

# set claim config
echo "Set config ✅"
cleos push action claim.pomelo setconfig '{"config":["ok", "login.eosn", "app.pomelo", "match.pomelo", ["shufti"], 180]}' -p claim.pomelo

#  set claim
echo "Set grant1 claim ✅"
cleos push action claim.pomelo setclaim '[101, grant1, ["900.0000 EOS","eosio.token"]]' -p claim.pomelo
echo "Set grant2 claim ✅"
cleos push action claim.pomelo setclaim '[101, grant2, ["100.0000 EOS","eosio.token"]]' -p claim.pomelo
# echo "Set grant1 claim ✅"
# cleos push action claim.pomelo setclaim '[101, grant1, ["1000.0000 USDT","tethertether"]]' -p claim.pomelo

# transfer tokens
echo "Transfering tokens for claim ✅"
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "for claim"

# bad claim
echo "Claim without auth - must fail ❌"
cleos push action claim.pomelo claim '[101, grant1]' -p user1

# bad claim
echo "Claim without approval - must fail ❌"
cleos push action claim.pomelo claim '[101, grant1]' -p prjman1

echo "Approve claim 1 ✅"
cleos push action claim.pomelo approve '[101, "grant1", true]' -p claim.pomelo

echo "Approve claim 2 ✅"
cleos push action claim.pomelo approve '[101, "grant2", true]' -p claim.pomelo

# claim funds without KYC - must fail
echo "Claim without KYC - must fail ❌"
cleos push action claim.pomelo claim '[101, grant1]' -p prjman1

# set KYC social
echo "Pass KYC ✅"
cleos push action login.eosn social '[prjman1.eosn, shufti]' -p prjman1.eosn

# claim funds with KYC
echo "Claim funds for grant1 ✅"
cleos push action claim.pomelo claim '[101, grant1]' -p prjman1
echo "Claim funds for grant2 ✅"
cleos push action claim.pomelo claim '[101, grant2]' -p prjman1

# sleep to avoid duplicate trx
sleep 1
echo "Claim funds for grant1 again - must fail ❌"
cleos push action claim.pomelo claim '[101, grant1]' -p prjman1

# sleep to avoid duplicate trx
sleep 1
echo "Transfer funds from the vault ✅"
# transfer funds from the vault
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "for claim"

#  set claim
# echo "Set grant1 claim ✅"
# cleos push action claim.pomelo setclaim '[101, grant1, ["900.0000 EOS","eosio.token"]]' -p claim.pomelo

# reclaim funds - must fail
echo "Reclaim funds with wrong authority - must fail ❌"
cleos push action claim.pomelo reclaim '[101, grant1]' -p prjman1

# reclaim funds
echo "Reclaim funds back to the vault ✅"
cleos push action claim.pomelo reclaim '[101, grant1]' -p claim.pomelo