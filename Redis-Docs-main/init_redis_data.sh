#!/bin/bash
redis-cli <<EOF
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
EOF
