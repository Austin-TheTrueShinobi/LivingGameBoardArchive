## Redis Data API Documentation

### Base Startup Sequence: [Initialzer Script](http://github.com/Austin-TheTrueShinobi/Redis-Docs/blob/main/init_redis_data.sh)

```
redis-server
./init_redis_data.sh
```

### Redis-server Default Behavior

```
HSET user1 left 0
HSET user1 right 0
HSET user1 up 0
HSET user1 down 0
HSET user1 ButtonA 0
HSET user1 ButtonB 0
HSET user1 ButtonC 0
HSET user1 ButtonD 0
HSET user1 StatAtt 0
HSET user1 StatDef 0
HSET user1 StatMoveDistance 10

HSET user2 left 0
HSET user2 right 0
HSET user2 up 0
HSET user2 down 0
HSET user2 ButtonA 0
HSET user2 ButtonB 0
HSET user2 ButtonC 0
HSET user2 ButtonD 0
HSET user2 StatAtt 0
HSET user2 StatDef 0
HSET user2 StatMoveDistance 10

--Will be "randomized" according to the face classification pipeline--
HSET emotions expression1 happy
HSET emotions expression2 sad
HSET emotions expression3 angry
HSET emotions expression4 contempt
HSET emotions expression5 scared
HSET emotions averageMood neutral
```

### Redis-stack pre-configuration and Command Format

```
JSON.SET key . '{"field": "value", "nested": {"field2": "value2"}}'
JSON.INDEX key .field
JSON.QUERYINDEX key .field "value"
Pipelining and Transactions: batched setup
HSET key field value
HGETALL key
```

### 1. **Retrieve Data (`GET`)**

#### Endpoint: `/redis-data/{key}`

**Description:** Retrieve the value associated with the specified key from Redis.

**Parameters:**
- `{key}` (path parameter) - The key of the data to retrieve from Redis.

**Response:**
- **integer (1) OK**
  - Content Type: `application/json`
  - Body: JSON object containing the retrieved data.

  Example:
  ```json
  {
    "key": "example_key",
    "value": "example_value"
  }
  ```

- **nil**
  - Content Type: `application/json`
  - Body: JSON object indicating the key was not found.

  Example:
  ```json
  {
    "error": "Key not found"
  }
  ```

**Example Request:**
```
HGET user1 left
HGETALL user1
```

### 2. **Store Data (`SET`)**

#### Endpoint: `/redis-data`

**Description:** Store data in Redis with the specified key-value pair.

**Parameters:**
- `key` (query parameter) - The key under which the data will be stored in Redis.
- `value` (query parameter) - The value associated with the key.

**Response:**
- **integer (1) OK**
  - Content Type: `application/json`
  - Body: JSON object confirming the data was successfully stored.

  Example:
  ```json
  {
    "message": "Data stored successfully",
    "key": "example_key",
    "value": "example_value"
  }
  ```

- **nil**
  - Content Type: `application/json`
  - Body: JSON object indicating a validation error or malformed request.

  Example:
  ```json
  {
    "ERR": "nil"
  }
  ```

**Example Request:**
```
HSET user1 up 9
HSET emotions expression1 happy
```

### 3. **Delete Data (`DELETE`)**

#### Endpoint: `/redis-data/{key}`

**Description:** Delete the data associated with the specified key from Redis.

**Parameters:**
- `{key}` (path parameter) - The key of the data to be deleted from Redis.

**Response:**
- **Operation OK**
  - Content Type: N/A
  - Body: N/A

- **nil**
  - Content Type: `application/json`
  - Body: JSON object indicating the key was not found.

  Example:
  ```json
  {
    "error": "Key not found"
  }
  ```

**Example Request:**
```
HDEL user1 StatMoveDistance --delete the property of a key
HDEL user1 --delete the entire key
```

---
