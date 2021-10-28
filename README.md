# 🍈 Pomelo Claim - EOSIO Smart Contract

## Usage

### `@admin`

```bash
# configure app
cleos push action claim.pomelo setconfig '{"config":["ok", "app.pomelo", "match.pomelo"]}' -p claim.pomelo

# transfer matching pool tokens from the vault
cleos transfer match.pomelo claim.pomelo "1000.0000 EOS" "grant:grant1"
cleos transfer match.pomelo claim.pomelo "1000.0000 USDT" "grant:grant1" --contract tethertether

# disable claims
cleos push action claim.pomelo setconfig '{"config":["disabled", "app.pomelo", "match.pomelo"]}' -p claim.pomelo
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
- [ACTION `claimlog`](#action-claimlog)
- [NOTIFIER `on_transfer`](#notifier-on_transfer)

## SINGLETON `config`

- `{name} status` - contract status `ok`/`disabled`
- `{name} pomelo_app` - Pomelo app account
- `{name} pomelo_match` - Pomelo vault account

### example

```json
{
    "status": "ok",
    "pomelo_app": "app.pomelo",
    "pomelo_match": "match.pomelo"
}
```

## TABLE `claims`

- **scope**: `get_self() {name}`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byfundingacc` | 2                | i64        |
| `bycreated`    | 3                | i64        |
| `byexpires`    | 4                | i64        |

### params

- `{name} project_id` - grant/bounty ID (primary key)
- `{name} funding_account` - funding account eligible to claim
- `{vector<extended_asset>} tokens` - claimable tokens
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} expires_at` - expires at time

### example

```json
{
    "project_id": "grant1",
    "funding_account": "prjman1",
    "tokens": ["1000.0000 EOS@eosio.token", "1000.0000 USDT@tethertether"],
    "created_at": "2021-12-06T00:00:00",
    "expires_at": "2022-12-06T00:00:00"
}
```

## ACTION `setconfig`

- **authority**: `get_self()`

Set contract configuration

### params

- `{optional<config_row>} config` - configuration (reset if null)

### example

```bash
$ cleos push action claim.pomelo setconfig '{"config":["ok", "app.pomelo", "match.pomelo"]}' -p claim.pomelo
$ cleos push action claim.pomelo setconfig '{"config":null}' -p claim.pomelo
```

## ACTION `claim`

- **authority**: `account`

Claim funds

### params

- `{name} account` - account making the claim

### example

```bash
$ cleos push action claim.pomelo claim '["prjaccount"]' -p prjaccount
```

## ACTION `claimlog`

- **authority**: `get_self()`

Log claim action

### params

- `{name} account` - account that claimed funds
- `{name} project` - project id associated with the claim
- `{vector<asset> claimed}` - claimed funds

## NOTIFIER `on_transfer`

Transfer funds for a claim.

Only transfers from Pomelo vault are accepted with the following memo format:
`grant:{grant_id}` or `bounty:{bounty_id}`.

After the transfer, funds become claimable by the corresponding `funding_account` account for that grant.

### example

```bash
$ cleos transfer match.pomelo claim.pomelo "10000 EOS" "grant:grant1" -p match.pomelo
```