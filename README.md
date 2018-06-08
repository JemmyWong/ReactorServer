## **************************** Reactor Server ********************************* 
1. Based on reactor architecture
2. Support config file
3. Support log
4. Support timer
5. Unified event source: [ I/O Event, Timer Event](src/eventhandler.c), [ Signal Event](src/sig_handler.c)

## TODO:
1. - [ ] Higher concurrence
2. - [ ] Same socket fd handled by multiple threads
3. - [ ] Timer event
4. - [ ] Set non-blocking socket
5. - [ ] Thread cancel
6. - [ ] Support HTTP Get and POST request
7. - [ ] Support HTTPS
8. - [ ] Secure access based on IP and mask
9. - [ ] When there is no resource to accept a new connection
10. - [ ] Program safe exit

## BUG:
1. - [x] Signal event bug
