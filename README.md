# LiamPhone

- pico-phone is the core code C code which gets compiled for both the actual phone (Raspberry Pi Pico microcontroller, ARM cortex M0+) and the simulator
- mock-backend implements everything to complete the core pico-phone code to run outside the production hardware and establishes a server for mock-host
- mock-host is code to run on Windows which connects to the mock-backend to display the simulated screen output and take input
