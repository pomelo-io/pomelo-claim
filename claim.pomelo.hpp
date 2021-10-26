#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;


class [[eosio::contract("claim.pomelo")]] claimpomelo : public eosio::contract {
public:
    using contract::contract;

    claimpomelo(name rec, name code, datastream<const char*> ds)
      : contract(rec, code, ds)
      , _config(get_self(), get_self().value)
    {
        check( _config.exists(), "claim.pomelo: config not set" );
    };


    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status - ok/disabled
     * - `{name} pomelo_account` - Pomelo contract account (app.pomelo)
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "pomelo_account": "app.pomelo",
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name    status;
        name    pomelo_account;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;


    /**
     * ## TABLE `claims`
     *
     * *scope*: `get_self()`
     *
     * - `{name} project_id` - grant/bounty ID (primary key)
     * - `{name} funding_account` - funding account eligible to claim
     * - `{vector<extended_asset>} tokens` - claimable tokens
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *      "project_id": "grant1",
     *      "funding_account": "prjman1",
     *      "tokens": ["1000.0000 EOS@eosio.token", "1000.0000 USDT@tethertether"]
     *      "updated_at": "2021-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("claims")]] claims_row {
        name                    project_id;
        name                    funding_account;
        vector<extended_asset>  tokens;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return project_id.value; };
        uint64_t by_funding_account() const { return funding_account.value; };
    };
    typedef eosio::multi_index< "claims"_n, claims_row,
        indexed_by< "byfundingacc"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_funding_account> >
     > claims_table;


    /**
     * ## ACTION `setconfig`
     *
     * Set config
     *
     * ### params
     *
     * - `{config_row} config` - contract config
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo setconfig '{"status": "ok", "pomelo_account": "app.pomelo"}' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const optional<config_row> config );


    /**
     * ## ACTION `claim`
     *
     * Claim allocated funds
     *
     * ### params
     *
     * - `{name} account` - grant funding account
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo claim '["prjman1"]' -p prjman1
     * ```
     */
    [[eosio::action]]
    void claim( const name account );

    /**
     * ## TRANSFER NOTIFY HANDLER `on_transfer`
     *
     * Process incoming transfer
     *
     * ### params
     *
     * - `{name} from` - from EOS account (donation sender)
     * - `{name} to` - to EOS account (process only incoming)
     * - `{asset} quantity` - quantity received
     * - `{string} memo` - transfer memo, i.e. "grant:myproject"
     *
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

private:
    config_table _config;

    void add_tokens( const name project_id, const name funding_account, const extended_asset ext_quantity);

    void transfer( const name to, const extended_asset value, const string memo );

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

};
