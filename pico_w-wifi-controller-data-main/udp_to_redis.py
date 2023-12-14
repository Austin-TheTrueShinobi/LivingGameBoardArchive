import socket
import redis

# Redis connection setup
redis_host = 'localhost'  # Redis server address
redis_port = 6379          # Redis server port
redis_db = 0               # Redis database index

# UDP server setup
udp_host = '0.0.0.0'      # Listen on all available interfaces
udp_port = 4444            # UDP port to listen on
buffer_size = 1024         # Size of the receive buffer

# Create a UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Allow the socket to be reused to avoid port conflictions
udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
udp_socket.bind((udp_host, udp_port))

# Create the required redis keys
user_hash1 = "user1"  # Redis key prefix for user1
user_hash2 = "user2"  # Redis key prefix for user2
user_initiated = 0

# Connect to Redis
redis_client = redis.StrictRedis(host=redis_host, port=redis_port, db=redis_db)
print(f"Listening for UDP packets on port {udp_port}...")

# sets the first user controller to the first received payload
while True:
    # Attach the recieved udp payload to the corrosponding byte key fields
    data, addr = udp_socket.recvfrom(buffer_size)  # Receive UDP data and sender's address
    data_str = data.decode('utf-8')               # Assuming data is in UTF-8 encoding
    updated_string = ''.join([chr(byte) for byte in data])  # Convert each byte to its corresponding ASCII character
    user_active = data_str[:7]  # Get the first 7 bytes of the bytes object to determine the user
    
    # set the first user
    if user_initiated = 0:
        user_alternate = user_active
        
    # alternate method for extracting bytes
    # byte_array = bytearray.fromhex(data_str)  # Convert hex string to a byte array
    # data = [byte_array[i] for i in range(7, 11)]  # Extract bytes 7-10 from the byte array

    # print the sender LAN address in IPV4
    print(f"Received UDP packet from {addr}")

    # Store data into Redis
    try:
        # select the user from the transmitted signal
        if user_active = user_alternate: # user1
            # push the entire payload for testing
            redis_client.lpush('udp_packets', data_str)  # Store data into a Redis list called 'udp_packets'
    
            # set the key-value according to its hash value in redis
            redis_client.hset(f"{user_hash1}", "up", updated_string[7])
            redis_client.hset(f"{user_hash1}", "down", updated_string[8])
            redis_client.hset(f"{user_hash1}", "left", updated_string[9])
            redis_client.hset(f"{user_hash1}", "right", updated_string[10])
        else: # user2
            # push the entire payload for testing
            redis_client.lpush('udp_packets', data_str)  # Store data into a Redis list called 'udp_packets'
    
            # set the key-value according to its hash value in redis
            redis_client.hset(f"{user_hash2}", "up", updated_string[7])
            redis_client.hset(f"{user_hash2}", "down", updated_string[8])
            redis_client.hset(f"{user_hash2}", "left", updated_string[9])
            redis_client.hset(f"{user_hash2}", "right", updated_string[10])

        print("Data stored in Redis.")
    except Exception as e:
        print(f"Failed to store data in Redis: {e}")
    user_initiated = 1    

# Close the UDP socket (this part of the code is unreachable in this example)
udp_socket.close()
