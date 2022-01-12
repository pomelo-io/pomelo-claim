# 🍈 Pomelo Claim - EOSIO Smart Contract

## Usage

### `@admin`

```bash
# configure app
cleos push action claim.pomelo setconfig '{"config":["ok", "login.eosn", "app.pomelo"]}' -p claim.pomelo

# set eligible claim amount
cleos push action claim.pomelo setclaim '[101, "grant1", ["1.0000 EOS", "eosio.token"]]' -p claim.pomelo

# admin approves claim
cleos push action claim.pomelo approve '[101, "grant1", true]' -p claim.pomelo

# cancel claim amount
cleos push action claim.pomelo cancel '[101, "grant1"]' -p claim.pomelo
```

### `@user`

```bash
# claim grant rewards
cleos push action claim.pomelo claim '["myaccount", grant1]]' -p myaccount
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
- [ACTION `claim`](#action-claim)
- [ACTION `setclaim`](#action-setclaim)
- [ACTION `approve`](#action-approve)
- [ACTION `cancel`](#action-cancel)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `claimlog`](#action-claimlog)
- [NOTIFIER `on_transfer`](#notifier-on_transfer)

## SINGLETON `config`

- `{name} status` - contract status - ok/disabled
- `{name} login_contract` - EOSN Login contract account (login.eosn)
- `{name} pomelo_app` - Pomelo contract account (app.pomelo)

### example

```json
{
    "status": "ok",
    "login_contract": "login.eosn",
    "pomelo_app": "app.pomelo"
}
```

## TABLE `claims`

- **scope**: `{uint16_t} round_id`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byauthor`     | 2                | i64        |
| `byfunding`    | 3                | i64        |
| `byclaimed`    | 4                | i64        |
| `bycreated`    | 5                | i64        |
| `byexpires`    | 6                | i64        |

### params

- `{name} grant_id` - grant ID (primary key)
- `{name} author_user_id` - grant author user id for KYC check
- `{name} funding_account` - funding account eligible to claim
- `{bool} approved` - approved claim
- `{extended_asset} claim` - claim amount
- `{extended_asset} claimed` - claimed amount
- `{time_point_sec} claimed_at - claimed at timestamp
- `{time_point_sec} expires_at - claim expires at timestamp
- `{time_point_sec} created_at` - updated at timestamp

### example

```json
{
    "grant_id": "grant1",
    "author_user_id": "123.eosn",
    "funding_account": "myaccount",
    "approved": true,
    "claim": {"contract": "eosio.token", "quantity": "1000.0000 EOS"},
    "claimed": {"contract": "eosio.token", "quantity": "0.0000 EOS"},
    "claimed_at": "1970-01-01T00:00:00",
    "expires_at": "2022-12-06T00:00:00",
    "created_at": "2021-12-06T00:00:00"
}
```

## ACTION `claim`

Claim allocated funds

- **authority**: `funding_account` or `author_user_id` or `get_self()`

### params

- `{uint16_t} round_id` - round ID
- `{name} grant_id` - grant ID to claim funds

### example

```bash
$ cleos push action claim.pomelo claim '[101, "grant1"]' -p myaccount
```


## ACTION `setclaim`

Set claim allocation based on matching amounts

- **authority**: `get_self()`

### params

- `{uint16_t} round_id` - round ID
- `{name} grant_id` - grant ID to setclaim
- `{extended_asset} claim` - claim amount

### example

```bash
$ cleos push action claim.pomelo setclaim '[101, "grant1", ["1.0000 EOS", "eosio.token"]]' -p claim.pomelo
```

## ACTION `approve`

Approve/disapprove the claim

- **authority**: `get_self()`

### params

- `{uint16_t} round_id` - round ID
- `{name} grant_id` - grant ID to setclaim
- `{bool} approved` - approved flag (true/false)

### example

```bash
$ cleos push action claim.pomelo approve '[101, "grant1", true]' -p claim.pomelo
```

## ACTION `cancel`

Remove claim

- **authority**: `get_self()`

### params

- `{uint16_t} round_id` - round ID
- `{name} grant_id` - grant ID to cancel

### example

```bash
$ cleos push action claim.pomelo cancel '[101, "grant1"]' -p claim.pomelo
```

## ACTION `setconfig`

- **authority**: `get_self()`

Set contract configuration

### params

- `{optional<config_row>} config` - configuration (reset if null)

### example

```bash
$ cleos push action claim.pomelo setconfig '{"config": ["ok", "login.eosn", "app.pomelo"]}' -p claim.pomeloclaim.pomelo
$ cleos push action claim.pomelo setconfig '[null]' -p claim.pomelo
```

## ACTION `claimlog`

Log claim

### params

- `{uint16_t} round_id` - round ID
- `{name} grant_id` - grant ID
- `{name} author_user_id` - grant author user id for KYC check
- `{name} funding_account` - funding account eligible to claim
- `{extended_asset} claimed` - claimed funds