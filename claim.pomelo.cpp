#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include <pomelo.app/app.pomelo.hpp>
#include <eosn.login/login.eosn.hpp>

#include "claim.pomelo.hpp"

// error messages
static const string CLAIM_MAINTENANCE = "claim.pomelo: contract is under maintenance";

// variables
static const name KYC_PROVIDER = "shufti"_n;
static const uint32_t CLAIM_PERIOD_DAYS = 90;

[[eosio::action]]
void claimpomelo::setconfig( const optional<config_row> config )
{
    require_auth( get_self() );

    config_table _config( get_self(), get_self().value );
    if ( !config ) {
        _config.remove();
        return;
    }
    check( is_account(config->pomelo_app), "claim.pomelo: invalid pomelo app account");
    check( is_account(config->login_contract), "claim.pomelo: invalid login contract");

    _config.set(*config, get_self());
}

[[eosio::action]]
void claimpomelo::setclaim( const uint16_t round_id, const name grant_id, const extended_asset claim )
{
    require_auth( get_self() );
    check_status();

    config_table _config( get_self(), get_self().value );
    claims_table _claims( get_self(), round_id );
    const name POMELO_APP = _config.get().pomelo_app;

    // get author & funding directly from pomelo app
    pomelo::grants_table _grants( POMELO_APP, POMELO_APP.value );
    auto grant = _grants.get( grant_id.value, "claim.pomelo::setclaim: [grant_id] does not exists in pomelo.app::grants");
    check( grant.status == "published"_n, "claim.pomelo::setclaim: [grant.status] must be published");

    // validate grant has received matching amounts
    pomelo::match_table _match( POMELO_APP, round_id );
    auto match = _match.get( grant_id.value, "claim.pomelo::setclaim: [grant_id] does not exists in pomelo.app::match");
    check( match.total_users > 0, "claim.pomelo::setclaim: [match.total_users] has no contributing users");
    check( match.sum_value > 0, "claim.pomelo::setclaim: [match.sum_value] has no contributions");

    // no duplicates grant claims
    check( _claims.find( grant_id.value ) == _claims.end(), "claim.pomelo::setclaim: [grant_id] already exists");

    _claims.emplace( get_self(), [&]( auto& row ) {
        row.funding_account = grant.funding_account;
        row.author_user_id = grant.author_user_id;
        row.grant_id = grant_id;
        row.created_at = current_time_point();
        row.expires_at = time_point_sec(current_time_point().sec_since_epoch() + CLAIM_PERIOD_DAYS * 24 * 60 * 60);
        row.claim = claim;
    });
}

[[eosio::action]]
void claimpomelo::claim( const uint16_t round_id, const name grant_id )
{
    // ** authority check is later
    check_status();

    claims_table _claims( get_self(), round_id );
    const auto& claim = _claims.get( grant_id.value, "claim.pomelo::claim: [grant_id] does not exists");

    // validate claim
    // ==============
    // authority by [author or funding or self]
    if ( !has_auth(claim.funding_account) && !has_auth(claim.author_user_id) && !has_auth(get_self()) ) {
        check( false, "claim.pomelo::claim: invalid claiming account (claim must be authorized by [funding_account] or [author_user_id] )");
    }
    check( claim.claimed_at.sec_since_epoch() == 0, "claim.pomelo::claim: grant has already claimed");
    check( claim.claim.quantity.amount != 0, "claim.pomelo::claim: grant has already claimed");
    check( claim.approved, "claim.pomelo::claim: grant claim has not yet been approved");
    check_kyc( claim.author_user_id );

    // transfer matching prize to funding account
    transfer( claim.funding_account, claim.claim, "🍈 " + grant_id.to_string() + " matching prize received via Pomelo.io" );

    // update claim status
    _claims.modify( claim, get_self(), [&]( auto& row ) {
        row.claimed_at = current_time_point();
        row.claimed = claim.claim;
        row.claim.quantity.amount = 0;
    });

    // logging
    claimlog_action claim_log( get_self(), { get_self(), "active"_n });
    claim_log.send( round_id, grant_id, claim.funding_account, claim.author_user_id, claim.claim );
}

[[eosio::action]]
void claimpomelo::reclaim( const uint16_t round_id, const name grant_id )
{
    require_auth( get_self() );

    claims_table _claims( get_self(), round_id );
    const auto& claim = _claims.get( grant_id.value, "claim.pomelo::claim: [grant_id] does not exists");
    _claims.erase( claim );
}


[[eosio::action]]
void claimpomelo::approve( const uint16_t round_id, const name grant_id, const bool approved )
{
    require_auth( get_self() );

    claims_table _claims( get_self(), round_id );
    const auto& claim = _claims.get( grant_id.value, "claim.pomelo::approve: [grant_id] does not exists");
    check( claim.approved != approved, "claim.pomelo::approve: [approved] was not modified");
    _claims.modify( claim, get_self(), [&]( auto& row ) {
        row.approved = approved;
    });
}

void claimpomelo::check_status()
{
    config_table _config(get_self(), get_self().value);
    check( _config.exists() && _config.get().status == "ok"_n, CLAIM_MAINTENANCE);
}

[[eosio::action]]
void claimpomelo::claimlog( const uint16_t round_id, const name grant_id, const name funding_account, const name author_user_id, const extended_asset claimed )
{
    require_auth( get_self() );

    require_recipient( funding_account );
    require_recipient( author_user_id );
}

void claimpomelo::transfer( const name to, const extended_asset value, const string memo )
{
    const auto balance = sx::utils::get_balance(value.get_extended_symbol(), get_self()).quantity;
    check( balance >= value.quantity, "claim.pomelo::transfer: not enough balance to transfer, please contact Pomelo admin");

    eosio::token::transfer_action transfer( value.contract, { get_self(), "active"_n });
    transfer.send( get_self(), to, value.quantity, memo );
}

void claimpomelo::check_kyc( const name author_user_id )
{
    config_table _config(get_self(), get_self().value);
    const auto& config = _config.get();

    eosn::login::users_table _users( config.login_contract, config.login_contract.value );
    const auto& user = _users.get( author_user_id.value, "claim.pomelo::check_kyc: [author_user_id] not found");
    check( user.socials.count(KYC_PROVIDER), "claim.pomelo::check_kyc: project [author_user_id] needs to pass KYC first" );
}

template <typename T>
void claimpomelo::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}
