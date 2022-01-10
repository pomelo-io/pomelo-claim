#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("claim.pomelo")]] claimpomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status - ok/disabled
     * - `{name} login_contract` - EOSN Login contract account (login.eosn)
     * - `{name} pomelo_app` - Pomelo contract account (app.pomelo)
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "login_contract": "login.eosn",
     *     "pomelo_app": "app.pomelo"
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name            status;
        name            login_contract;
        name            pomelo_app;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `claims`
     *
     * *scope*: `{uint16_t} round_id`
     *
     * - `{name} grant_id` - grant ID (primary key)
     * - `{name} author_user_id` - grant author user id for KYC check
     * - `{name} funding_account` - funding account eligible to claim
     * - `{bool} approved` - approved claim
     * - `{extended_asset} claim` - claim amount
     * - `{extended_asset} claimed` - claimed amount
     * - `{time_point_sec} claimed_at - claimed at timestamp
     * - `{time_point_sec} expires_at - claim expires at timestamp
     * - `{time_point_sec} created_at` - updated at timestamp
     *
     * ### example
     *
     * ```json
     * {
     *      "grant_id": "grant1",
     *      "author_user_id": "123.eosn",
     *      "funding_account": "myaccount",
     *      "approved": true,
     *      "claim": {"contract": "eosio.token", "quantity": "1000.0000 EOS"},
     *      "claimed": {"contract": "eosio.token", "quantity": "0.0000 EOS"},
     *      "claimed_at": "1970-01-01T00:00:00",
     *      "expires_at": "2022-12-06T00:00:00",
     *      "created_at": "2021-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("claims")]] claims_row {
        name                    grant_id;
        name                    author_user_id;
        name                    funding_account;
        bool                    approved;
        extended_asset          claim;
        extended_asset          claimed;
        time_point_sec          claimed_at;
        time_point_sec          created_at;
        time_point_sec          expires_at;

        uint64_t primary_key() const { return grant_id.value; };
        uint64_t by_author_user_id() const { return author_user_id.value; };
        uint64_t by_funding_account() const { return funding_account.value; };
        uint64_t by_claimed() const { return claimed_at.sec_since_epoch(); };
        uint64_t by_created() const { return created_at.sec_since_epoch(); };
        uint64_t by_expires() const { return expires_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "claims"_n, claims_row,
        indexed_by< "byauthor"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_author_user_id> >,
        indexed_by< "byfunding"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_funding_account> >,
        indexed_by< "byclaimed"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_claimed> >,
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
     * - **authority**: `funding_account` or `author_user_id` or `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID to claim funds
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo claim '[101, "grant1"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void claim( const uint16_t round_id, const name grant_id );

    /**
     * ## ACTION `reclaim`
     *
     * Reclaim allocated funds back to Pomelo vault account
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID to reclaim
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo reclaim '[101, "grant1"]' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void reclaim( const uint16_t round_id, const name grant_id );

    /**
     * ## ACTION `setclaim`
     *
     * Set claim allocation based on matching amounts
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID to setclaim
     * - `{extended_asset} claim` - claim amount
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo setclaim '[101, "grant1", ["1.0000 EOS", "eosio.token"]]' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void setclaim( const uint16_t round_id, const name grant_id, const extended_asset claim );

    /**
     * ## ACTION `setclaim`
     *
     * Set claim allocation based on matching amounts
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID to setclaim
     * - `{bool} approved` - approved flag (true/false)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo setclaim '[101, "grant1", true]' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void approve( const uint16_t round_id, const name grant_id, const bool approved );

    /**
     * ## ACTION `claimlog`
     *
     * Log claim
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID
     * - `{name} author_user_id` - grant author user id for KYC check
     * - `{name} funding_account` - funding account eligible to claim
     * - `{extended_asset} claimed` - claimed funds
     */
    [[eosio::action]]
    void claimlog( const uint16_t round_id, const name grant_id, const name funding_account, const name author_user_id, const extended_asset claimed );
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &claimpomelo::claimlog>;

private:
    void add_tokens( const name project_id, const name author_user_id, const name funding_account, const extended_asset ext_quantity, const uint64_t claim_period_days);

    void transfer( const name to, const extended_asset value, const string memo );
    void check_status();

    void check_kyc( const name author_user_id );

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

};
