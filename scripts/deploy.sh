#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create account
cleos create account eosio claim.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio match.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio app.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fee.pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio login.eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio oracle.defi EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio tethertether EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjbounty1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# contract
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract tethertether ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract login.eosn ./include/eosn.login login.eosn.wasm login.eosn.abi
cleos set contract oracle.defi ./include/oracle.defi oracle.defi.wasm oracle.defi.abi
cleos set contract app.pomelo ./include/pomelo.app app.pomelo.wasm app.pomelo.abi
cleos set contract claim.pomelo . claim.pomelo.wasm claim.pomelo.abi

# @eosio.code permission
cleos set account permission claim.pomelo active --add-code
cleos set account permission app.pomelo active --add-code
cleos set account permission login.eosn active --add-code
cleos set account permission eosn active login.eosn --add-code

# create tokens
cleos push action eosio.token create '["eosio", "10000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 EOS", "init"]' -p eosio
cleos push action tethertether create '["eosio", "100000000.0000 USDT"]' -p tethertether
cleos push action tethertether issue '["eosio", "10000000.0000 USDT", "init"]' -p eosio

# transfer tokens
cleos transfer eosio match.pomelo "1000000.0000 EOS" ""
cleos transfer eosio match.pomelo "1000000.0000 USDT" "" --contract tethertether

# set price in oracle contract
cleos push action oracle.defi setprice '[1, ["4,EOS", "eosio.token"], 4, 50000]' -p oracle.defi
