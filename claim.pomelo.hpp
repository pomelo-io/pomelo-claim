#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("claim.pomelo")]] claimpomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## SINGLETON `config`
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
     * - **scope**: `{uint16_t} round_id`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `byauthor`     | 2                | i64        |
     * | `byfunding`    | 3                | i64        |
     *
     * ### params
     *
     * - `{name} grant_id` - grant ID (primary key)
     * - `{name} author_user_id` - grant author user id for KYC check
     * - `{name} funding_account` - funding account eligible to claim
     * - `{bool} approved` - approved claim
     * - `{extended_asset} claim` - claim amount
     * - `{extended_asset} claimed` - claimed amount
     * - `{checksum256} trx_id` - transaction ID
     * - `{time_point_sec} claimed_at` - claimed at timestamp
     * - `{time_point_sec} expires_at` - claim expires at timestamp
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
     *      "trx_id": "8e5ce72555af183b98a0180b2da507c16fd60f2831854129b289876ecc41b7e2",
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
        checksum256             trx_id;
        time_point_sec          claimed_at;
        time_point_sec          created_at;
        time_point_sec          expires_at;

        uint64_t primary_key() const { return grant_id.value; };
        uint64_t by_author_user_id() const { return author_user_id.value; };
        uint64_t by_funding_account() const { return funding_account.value; };
    };
    typedef eosio::multi_index< "claims"_n, claims_row,
        indexed_by< "byauthor"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_author_user_id> >,
        indexed_by< "byfunding"_n, const_mem_fun<claims_row, uint64_t, &claims_row::by_funding_account> >
    > claims_table;

    /**
     * ## ACTION `setconfig`
     *
     * Set config
     *
     * ### params
     *
     * - `{optional<config_row>} config` - configuration (reset if null)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo setconfig '{"config":["ok", "login.eosn", "app.pomelo"]}' -p claim.pomelo
     * $ cleos push action claim.pomelo setconfig '[null]' -p claim.pomelo
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
     * - `{bool} [staked=false]` - (optional) to receive matching prize as staked (delegated to CPU)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo claim '[101, "grant1", null]' -p myaccount
     *
     * // receive as staked
     * $ cleos push action claim.pomelo claim '[101, "grant1", true]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void claim( const uint16_t round_id, const name grant_id, const optional<bool> staked );

    /**
     * ## ACTION `cancel`
     *
     * Remove claim
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round ID
     * - `{name} grant_id` - grant ID to cancel
     *
     * ### example
     *
     * ```bash
     * $ cleos push action claim.pomelo cancel '[101, "grant1"]' -p claim.pomelo
     * ```
     */
    [[eosio::action]]
    void cancel( const uint16_t round_id, const name grant_id );

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
     * ## ACTION `approve`
     *
     * Approve/disapprove the claim
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
     * $ cleos push action claim.pomelo approve '[101, "grant1", true]' -p claim.pomelo
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

    // action wrappers
    using setconfig_action = eosio::action_wrapper<"setconfig"_n, &claimpomelo::setconfig>;
    using claim_action = eosio::action_wrapper<"claim"_n, &claimpomelo::claim>;
    using cancel_action = eosio::action_wrapper<"cancel"_n, &claimpomelo::cancel>;
    using setclaim_action = eosio::action_wrapper<"setclaim"_n, &claimpomelo::setclaim>;
    using approve_action = eosio::action_wrapper<"approve"_n, &claimpomelo::approve>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &claimpomelo::claimlog>;

private:
    void add_tokens( const name project_id, const name author_user_id, const name funding_account, const extended_asset ext_quantity, const uint64_t claim_period_days);

    void transfer( const name to, const extended_asset value, const string memo );
    void check_status();

    void check_kyc( const name author_user_id );

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
    checksum256 get_trx_id();

};
