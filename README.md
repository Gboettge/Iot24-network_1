# What my project mangages:
- up to 20 clients connection
- availability to read and wait for multiple clients simoutaniously through threading
- after connection starts a separate thread is created and handled within my client_handler() function

# Client handler will do the following:
- if message recieved is 0 bytes, (disconnect (do nothing))
- if message is bigger than buffer size (send ACK: message to big, and close connection)
- if message has and new line. Respond straight away (ack)
- if message is missing a new line. Save buffer and start a 10s wait thread, if 10s passed resent partial message

# how to run the files from terminal with help of cmake:
- bash commands:
    - cd build    
    - make          
    - ./server
    - ./client


