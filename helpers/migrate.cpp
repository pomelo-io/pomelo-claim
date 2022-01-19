
[[eosio::action]]
void claimpomelo::migrate( const uint16_t round_id )
{
    require_auth( get_self() );

    claims_table _claims( get_self(), round_id );
    claims_v2_table _claims_v2( get_self(), round_id );

    for ( auto claim : _claims_v2 ) {
        _claims.emplace( get_self(), [&]( auto& row ) {
            row.grant_id = claim.grant_id;
            row.author_user_id = claim.author_user_id;
            row.funding_account = claim.funding_account;
            row.claim = claim.claim;
            row.claimed = claim.claimed;
            row.claimed.contract = claim.claim.contract;
            row.claimed.quantity.symbol = claim.claim.quantity.symbol;
            row.approved = claim.approved;
            row.claimed_at = claim.claimed_at;
            row.created_at = claim.created_at;
            row.expires_at = claim.expires_at;
        });
    }
}

[[eosio::action]]
void claimpomelo::migrate2( const uint16_t round_id )
{
    require_auth( get_self() );

    claims_v2_table _claims_v2( get_self(), round_id );
    clear_table( _claims_v2, 400 );
}
