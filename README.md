## CREDIS
CRedis is my own implementation of Redis server using C Programming Language.
This implementation is based on [this tutorial](https://build-your-own.org/redis/).
You can use the client here : [client](https://github.com/agus-wesly/credis-client)

### Building and Running
```bash
make -B

./credis
```

## Table of Contents

1. [Primitive Commands](#primitive-commands)
2. [Sorted Set Commands](#sorted-set-commands)

## Primitive Commands

Primitive commands work with simple key-value pairs.

### `GET <key>`

Returns the value associated with the specified `key`.

**Example:**

```text
GET mykey
-> "value"
```

### `SET <key> <value>`

Sets the value of the specified `key`. If the key already exists, its value is updated.

**Example:**

```text
SET mykey "value"
-> OK
```

### `DEL <key>`

Deletes the specified `key` and returns the deleted value if it existed.

**Example:**

```text
DEL mykey
-> "value"
```

### `KEYS`

Returns a list of all keys currently stored in the database.

**Example:**

```text
KEYS
-> ["key1", "key2", "mykey"]
```

---

## Sorted Set Commands

Sorted sets are collections of unique keys ordered by a score. Credis supports the following operations on sorted sets.

### `ZADD <set_name> <score> <key>`

Adds a key with the specified score into the sorted set `set_name`. If the key exists, its score is updated.

**Example:**

```text
ZADD myset 10 "one"
-> 1
```

### `ZQUERY <set_name> <score> <key> <offset> <limit>`

Retrieves elements from the sorted set `set_name` where the score is greater than or equal to `<score>` and key is greater than or equal to `<key>`. Returns results based on the specified `offset` and `limit`.

**Example:**

```text
ZQUERY myset 5 "a" 0 10
-> ["a", "b", "c"]
```

### `ZRANK <set_name> <key>`

Returns the rank of the specified key within the sorted set. Rank starts at 1 for the smallest element.

**Example:**

```text
ZRANK myset "one"
-> 1
```

### `ZREM <set_name> <key>`

Removes the specified key from the sorted set `set_name`.

**Example:**

```text
ZREM myset "one"
-> OK
```

### `ZSCORE <set_name> <key>`

Returns the score of the specified key in the sorted set.

**Example:**

```text
ZSCORE myset "one"
-> 10
```
