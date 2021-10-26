#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build
blanc++ claim.pomelo.cpp -I include
cleos set contract claim.pomelo . claim.pomelo.wasm claim.pomelo.abi

# additional builds
if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    blanc++ ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/eosn.login/login.eosn.wasm" ]; then
    blanc++ ./include/eosn.login/login.eosn.cpp -I include -o include/eosn.login/login.eosn.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/oracle.defi/oracle.defi.wasm" ]; then
    blanc++ ./include/oracle.defi/oracle.defi.cpp -I include -o include/oracle.defi/oracle.defi.wasm --no-missing-ricardian-clause
fi

if [ ! -f "./include/pomelo.play/play.pomelo.wasm" ]; then
    blanc++ ./include/pomelo.play/play.pomelo.cpp -I include -o include/pomelo.play/play.pomelo.wasm --no-missing-ricardian-clause
fi

