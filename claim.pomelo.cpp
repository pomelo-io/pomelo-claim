#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include <pomelo.app/app.pomelo.hpp>

#include "claim.pomelo.hpp"


static const string CLAIM_INVALID_MEMO = "claim.pomelo::on_transfer: invalid memo";
static const string CLAIM_MAINTENANCE = "claim.pomelo: contract is under maintenance";

[[eosio::action]]
void claimpomelo::setconfig( const optional<config_row> config )
{
    require_auth( get_self() );

    claimpomelo::config_table config_( get_self(), get_self().value );
    if ( !config ) {
        config_.remove();
        return;
    }
    check( is_account(config->pomelo_app), "claim.pomelo: invalid pomelo app account");
    check( is_account(config->pomelo_match), "claim.pomelo: invalid pomelo vault account");
    check( config->claim_period_days > 0, "claim.pomelo: invalid claim period");

    config_.set(*config, get_self());
}


[[eosio::action]]
void claimpomelo::claim( const name account )
{
    require_auth( account );
    config_table _config(get_self(), get_self().value);
    check( _config.exists() && _config.get().status == "ok"_n, CLAIM_MAINTENANCE);

    claims_table claims( get_self(), get_self().value );
    auto index = claims.get_index<"byfundingacc"_n>();
    bool success = false;
    claimlog_action claim_log( get_self(), { get_self(), "active"_n });

    for( auto itr = index.find( account.value ); itr != index.end() && itr->funding_account == account; ){
        vector<asset> claimed;
        for(const auto token: itr->tokens){
            transfer( account, token, "🍈 " + itr->project_id.to_string() + " matching prize received via Pomelo.io" );
            claimed.push_back(token.quantity);
            success = true;
        }
        claim_log.send( account, itr->project_id, claimed );
        itr = index.erase( itr );
    }
    check( success, "claim.pomelo::claim: nothing to claim");
}

[[eosio::action]]
void claimpomelo::reclaim( const name project_id )
{
    config_table _config(get_self(), get_self().value);
    check( _config.exists() && _config.get().status == "ok"_n, CLAIM_MAINTENANCE);

    check( has_auth(_config.get().pomelo_match), "claim.pomelo::reclaim: funds can only be reclaimed by pomelo match account");

    claims_table claims( get_self(), get_self().value );
    const auto& claim = claims.get( project_id.value, "claim.pomelo::claim: no claimable funds for [project_id]");
    for(const auto token: claim.tokens){
        transfer( _config.get().pomelo_match, token, "🍈 " + project_id.to_string() + " matching prize reclaimed" );
    }
    claims.erase( claim );
}

[[eosio::action]]
void claimpomelo::claimlog( const name account, const name project, vector<asset> claimed )
{
    require_auth( get_self() );

}


void claimpomelo::transfer( const name to, const extended_asset tokens, const string memo )
{
    const auto balance = sx::utils::get_balance(tokens.get_extended_symbol(), get_self()).quantity;
    check( balance >= tokens.quantity, "claimpomelo::transfer: not enough balance to transfer");

    eosio::token::transfer_action transfer( tokens.contract, { get_self(), "active"_n });
    transfer.send( get_self(), to, tokens.quantity, memo );
}


[[eosio::on_notify("*::transfer")]]
void claimpomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );
    if( from == get_self() || from == "eosio.ram"_n) return;

    config_table _config(get_self(), get_self().value);
    check( _config.exists(), CLAIM_MAINTENANCE);
    const auto config = _config.get();
    check( from == config.pomelo_match, "claim.pomelo::on_transfer: only transfers from pomelo vault allowed");

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check( memo_parts.size() == 2, CLAIM_INVALID_MEMO);
    const name project_type = sx::utils::parse_name(memo_parts[0]);
    const name project_id = sx::utils::parse_name(memo_parts[1]);

    check(project_id.value, "claim.pomelo::on_transfer: invalid project id");

    name funding_account;
    if (project_type == "grant"_n) {
        pomelo::grants_table grants( config.pomelo_app, config.pomelo_app.value );
        funding_account = grants.get(project_id.value, "claim.pomelo::on_transfer: grant not found").funding_account;
    }
    else if (project_type == "bounty"_n) {
        pomelo::bounties_table bounties( config.pomelo_app, config.pomelo_app.value );
        funding_account = bounties.get(project_id.value, "claim.pomelo::on_transfer: bounty not found").funding_account;
    }
    else check( false, CLAIM_INVALID_MEMO);

    check( funding_account.value, "claim.pomelo::on_transfer: empty funding account");

    add_tokens(project_id, funding_account, extended_asset{ quantity, get_first_receiver()}, config.claim_period_days);
}


void claimpomelo::add_tokens( const name project_id, const name funding_account, const extended_asset ext_quantity, const uint64_t claim_period_days)
{
    claims_table claims( get_self(), get_self().value );

    const auto modify = [&]( auto& row ) {
        row.funding_account = funding_account;
        row.project_id = project_id;
        if( !row.created_at.sec_since_epoch()) row.created_at = current_time_point();
        row.expires_at = time_point_sec(current_time_point().sec_since_epoch() + claim_period_days * 24 * 60 * 60);
        bool added = false;
        for( auto& token: row.tokens){
            if( token.get_extended_symbol() == ext_quantity.get_extended_symbol()){
                token += ext_quantity;
                added = true;
                break;
            }
        }
        if( !added) row.tokens.push_back( ext_quantity );
    };

    auto it = claims.find( project_id.value );
    if( it == claims.end() ) {
        claims.emplace( get_self(), modify);
    } else {
        check( it->funding_account == funding_account, "claimpomelo::add_claim: funding account changed");
        claims.modify( it, same_payer, modify);
    }
}

template <typename T>
void claimpomelo::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}