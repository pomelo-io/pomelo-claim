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
      : contract(rec, code, ds) {};


    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status - ok/disabled
     * - `{name} login_contract` - EOSN Login contract account (login.eosn)
     * - `{name} pomelo_app` - Pomelo contract account (app.pomelo)
     * - `{name} pomelo_match` - Pomelo vault account where transfers come from (match.pomelo)
     * - `{set<name>} kyc_providers` - EOSN socials that are used for KYC
     * - `{uint32_t} claim_period_days` - Claim expiry period in days
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "login_contract": "login.eosn",
     *     "pomelo_app": "app.pomelo",
     *     "pomelo_match": "match.pomelo",
     *     "kyc_providers": ["shufti"],
     *     "claim_period_days": 180
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name            status;
        name            login_contract;
        name            pomelo_app;
        name            pomelo_match;
        vector<name>    kyc_providers;
        uint32_t        claim_period_days;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;


    /**
     * ## TABLE `claims`
     *
     * *scope*: `get_self()`
     *
     * - `{name} project_id` - grant/bounty ID (primary key)
     * - `{name} author_user_id` - grant author user id for KYC check
     * - `{name} funding_account` - funding account eligible to claim
     * - `{vector<extended_asset>} tokens` - claimable tokens
     * - `{time_point_sec} expires_at - claim expires at time
     * - `{time_point_sec} created_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *      "project_id": "grant1",
     *      "author_user_id": "prjman1.eosn",
     *      "funding_account": "prjman1",
     *      "tokens": ["1000.0000 EOS@eosio.token", "1000.0000 USDT@tethertether"],
     *      "expires_at": "2022-12-06T00:00:00"
     *      "created_at": "2021-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("claims")]] claims_row {
        name                    project_id;
        name                    author_user_id;
        name                    funding_account;
        vector<extended_asset>  tokens;
        time_point_sec          created_at;
        time_point_sec          expires_at;

        uint64_t primary_key() const { return project_id.value; };
        uint64_t by_funding_account() const { return funding_account.value; };
        uint64_t by_created() const { return created_at.sec_since_epoch(); };
        uint64_t by_expires() const { return expires_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "claims"_n, claims_row,
        indexed_by< "byfundingacc"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_funding_account> >,
        indexed_by< "bycreated"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_created> >,
        indexed_by< "byexpires"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_expires> >
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
     * $ cleos push action claim.pomelo setconfig '{"config":{"status": "ok", "pomelo_app": "app.pomelo", "pomelo_match": "match.pomelo"}}' -p claim.pomelo
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
     * - `{name} account` - funding account elibigle for the matching prize claim
     * - `{name} project_id` - id for a project to claim funds for
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo claim '[prjman1, grant1]' -p prjman1
     * ```
     */
    [[eosio::action]]
    void claim( const name account, const name grant_id );

    /**
     * ## ACTION `reclaim`
     *
     * Reclaim allocated funds back to Pomelo vault account
     *
     * ### params
     *
     * - `{name} project_id` - project id to reclaim
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo reclaim '["grant1"]' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void reclaim( const name project_id );

    /**
     * ## TRANSFER NOTIFY HANDLER `on_transfer`
     *
     * Process incoming transfer
     *
     * ### params
     *
     * - `{name} from` - from EOS account
     * - `{name} to` - to EOS account (process only incoming)
     * - `{asset} quantity` - quantity received
     * - `{string} memo` - transfer memo, i.e. "grant:myproject"
     *
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );


    /**
     * ## ACTION `claimlog`
     *
     * Log claim
     *
     * ### params
     *
     * - `{name} account` - account that claimed funds
     * - `{name} project` - project id associated with the claim
     * - `{vector<asset> claimed} - claimed funds
     *
     * ```
     */
    [[eosio::action]]
    void claimlog( const name account, const name project, vector<asset> claimed );
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &claimpomelo::claimlog>;

private:
    void add_tokens( const name project_id, const name author_user_id, const name funding_account, const extended_asset ext_quantity, const uint64_t claim_period_days);

    void transfer( const name to, const extended_asset value, const string memo );

    void check_kyc( const name author_user_id );

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

};
