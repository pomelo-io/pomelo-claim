#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# enable tokens
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo
cleos push action app.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p app.pomelo

# create and link eosn acc
cleos push action login.eosn create '["prjman1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn link '["prjman1.eosn", "prjman1", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p prjman1

# create matching round and start it
cleos push action app.pomelo setconfig '[0, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2022-05-25T20:00:00", "2021-05-19T20:00:00", "2022-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setgrant '["prjman1.eosn", "grant1", "prjman1", ["EOS", "USDT"]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn


# set claim config
cleos push action claim.pomelo setconfig '{"config":["ok", "app.pomelo", "match.pomelo"]}' -p claim.pomelo

# transfer funds from the vault
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "grant:grant1"
cleos transfer match.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether

# claim funds
cleos push action claim.pomelo claim '[prjman1]' -p prjman1

# sleep to avoid duplicate trx
sleep 1

# transfer funds from the vault
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "grant:grant1"
cleos transfer match.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether

# reclaim funds
cleos push action claim.pomelo reclaim '[grant1]' -p claim.pomelo