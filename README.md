## **************************** Reactor Server ********************************* 
1. Based on reactor architecture
2. Support config file
3. Support log
4. Support timer, based on heap
5. Unified event source: [I/O Event](src/handler_io.cc), [Signal Event](src/handler_sig.cc), [Timer Event](src/handler_timer.cc)

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
11. - [ ] Run in daemon model
12. - [ ] Rebuild to CC

## BUG:
1. - [x] Signal event bug