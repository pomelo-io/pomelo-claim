#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include <pomelo.app/app.pomelo.hpp>

#include "claim.pomelo.hpp"


static const string CLAIM_INVALID_MEMO = "claim.pomelo::on_transfer: invalid memo";

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

    config_.set(*config, get_self());
}


[[eosio::action]]
void claimpomelo::claim( const name account )
{
    require_auth( account );
    config_table _config(get_self(), get_self().value);
    check( _config.exists() && _config.get().status == "ok"_n, "claim.pomelo::on_transfer: contract is under maintenance");

    claims_table claims( get_self(), get_self().value );
    auto index = claims.get_index<"byfundingacc"_n>();
    bool claimed = false;

    for( auto itr = index.find( account.value ); itr != index.end() && itr->funding_account == account; ){
        for(const auto token: itr->tokens){
            transfer( account, token, "🍈 " + itr->project_id.to_string() + " claimed" );
            claimed = true;
        }
        itr = index.erase( itr );
    }
    check( claimed, "claim.pomelo::claim: nothing to claim");
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

    config_table _config(get_self(), get_self().value);
    check( _config.exists(), "claimpomelo::on_transfer: config not set");

    const auto config = _config.get();
    if( from == get_self() || from == "eosio.ram"_n) return;
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

    add_tokens(project_id, funding_account, extended_asset{ quantity, get_first_receiver()});
}


void claimpomelo::add_tokens( const name project_id, const name funding_account, const extended_asset ext_quantity)
{
    claims_table claims( get_self(), get_self().value );

    const auto modify = [&]( auto& row ) {
        row.funding_account = funding_account;
        row.project_id = project_id;
        row.updated_at = current_time_point();
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
        check( it->funding_account == funding_account, "claimpomelo::add_claim: wrong funding account");
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