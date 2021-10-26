# 🍈 Pomelo Claim - EOSIO Smart Contract

## Usage

### `@admin`

```bash
# configure app
cleos push action claim.pomelo setconfig '{"config":["ok", "app.pomelo", "vault.pomelo"]}' -p claim.pomelo

# transfer matching pool tokens from the vault
cleos transfer vault.pomelo claim.pomelo "1000.0000 EOS" "grant:grant1"
cleos transfer vault.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether

# disable claims
cleos push action claim.pomelo setconfig '{"config":["disabled", "app.pomelo", "vault.pomelo"]}' -p claim.pomelo
```

### `@user`
```bash
# claim project funding
cleos push action claim.pomelo claim '["prjaccount"]]' -p prjaccount

```

## Testing

```bash
# build contracts
$ ./scripts/build.sh

# restart node, create EOSIO users, deploy contracts, issue tokens
$ ./scripts/restart

# run basic tests
$ ./scripts/test.sh
```

## Table of Content

- [SINGLETON `config`](#singleton-config)
- [TABLE `claims`](#table-claims)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `claim`](#action-claim)
- [NOTIFIER `on_transfer`](#notifier-on_transfer)

## SINGLETON `config`

- `{name} status` - contract status `ok`/`disabled`
- `{name} pomelo_app` - Pomelo app account
- `{name} pomelo_vault` - Pomelo vault account

### example

```json
{
    "status": "ok",
    "pomelo_app": "app.pomelo",
    "pomelo_vault": "vault.pomelo"
}
```

## TABLE `claims`

- **scope**: `get_self() {name}`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byfundingacc` | 2                | i64        |

### params

- `{name} project_id` - grant/bounty ID (primary key)
- `{name} funding_account` - funding account eligible to claim
- `{vector<extended_asset>} tokens` - claimable tokens
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "project_id": "grant1",
    "funding_account": "prjman1",
    "tokens": ["1000.0000 EOS@eosio.token", "1000.0000 USDT@tethertether"],
    "updated_at": "2021-12-06T00:00:00"
}
```

## ACTION `setconfig`

- **authority**: `get_self()`

Set contract configuration

### params

- `{optional<config_row>} config` - configuration (reset if null)

### example

```bash
$ cleos push action claim.pomelo setconfig '{"config":["ok", "app.pomelo", "vault.pomelo"]}' -p claim.pomelo
$ cleos push action claim.pomelo setconfig '{"config":null}' -p claim.pomelo
```

## ACTION `claim`

- **authority**: `account`

Claim funds

### params

- `{name} account` - account funds belong to

### example

```bash
$ cleos push action claim.pomelo claim '["prjaccount"]' -p prjaccount
```

## NOTIFIER `on_transfer`

Transfer funds for a claim.
Only transfers from Pomelo vault are accepted with the following memo format:
`grant:{grant_id}` or `bounty:{bounty_id}`.
After the transfer funds become claimable by the corresponding `funding_account` from Pomelo `bounties` or `grants` table

### example

```bash
$ cleos transfer vault.pomelo claim.pomelo "10000 EOS" "grant:grant1" -p vault.pomelo
```