# 🍈 Pomelo Claim - EOSIO Smart Contract

## Usage

### `@user`

```bash
# claim grant rewards (authorized by funding account)
cleos push action claim.pomelo claim '[101, grant1]]' -p myaccount
```

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

## Build

```bash
$ cdt-cpp claim.pomelo.cpp -I include
```

## HTTP requests

#### all claims for `round_id`

```curl
curl -X 'POST' \
  "https://eos.eosn.io/v1/chain/get_table_rows" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "code": "claim.pomelo",
  "scope": 101,
  "table": "claims",
  "json": true,
  "limit": 100
}' | jq .
```

#### by `grant_id`

```curl
curl -X 'POST' \
  "https://eos.eosn.io/v1/chain/get_table_rows" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "code": "claim.pomelo",
  "scope": 101,
  "table": "claims",
  "json": true,
  "lower_bound": "mygrant",
  "upper_bound": "mygrant"
}' | jq .
```

#### by `author_user_id` (EOSN account)

```curl
curl -X 'POST' \
  "https://eos.eosn.io/v1/chain/get_table_rows" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "code": "claim.pomelo",
  "scope": 101,
  "table": "claims",
  "json": true,
  "lower_bound": "123.eosn",
  "upper_bound": "123.eosn",
  "index_position": "secondary",
  "key_type": "name"
}' | jq .
```

#### by `funding_account` (EOS account)

```curl
curl -X 'POST' \
  "https://eos.eosn.io/v1/chain/get_table_rows" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "code": "claim.pomelo",
  "scope": 101,
  "table": "claims",
  "json": true,
  "lower_bound": "myaccount",
  "upper_bound": "myaccount",
  "index_position": "tertiary",
  "key_type": "name"
}' | jq .
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
| `bygrant`      | `primary`   (1)  | name       |
| `byauthor`     | `secondary` (2)  | name       |
| `byfunding`    | `tertiary`  (3)  | name       |

### params

- `{name} grant_id` - grant ID (primary key)
- `{name} author_user_id` - grant author user id for KYC check
- `{name} funding_account` - funding account eligible to claim
- `{bool} approved` - approved claim
- `{extended_asset} claim` - claim amount
- `{extended_asset} claimed` - claimed amount
- `{time_point_sec} claimed_at` - claimed at timestamp
- `{time_point_sec} expires_at` - claim expires at timestamp
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
- `{bool} [staked=false]` - to receive matching prize as staked (delegated to CPU)

### example

```bash
$ cleos push action claim.pomelo claim '[101, "grant1", false]' -p myaccount

// receive as staked
$ cleos push action claim.pomelo claim '[101, "grant1", true]' -p myaccount
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
