# README for aesdsocket

# aesdsocket

## Overview
The `aesdsocket` program is a socket-based application designed to handle incoming client connections on port 9000. It allows clients to send data, which is then appended to a file located at `/var/tmp/aesdsocketdata`. The program also returns the contents of this file to the client upon request. It includes logging capabilities to track connections and disconnections via syslog, and it gracefully handles termination signals.

## Features
- Creates a TCP socket and binds it to port 9000.
- Listens for incoming client connections.
- Accepts connections and logs messages to syslog.
- Receives data packets from clients and appends them to `/var/tmp/aesdsocketdata`.
- Returns the contents of the file to the client.
- Handles SIGINT and SIGTERM signals for graceful shutdown.

## Compilation
To compile the `aesdsocket` program, navigate to the `server` directory and run the following command:

```bash
make
```

This will generate the executable `aesdsocket` in the same directory.

## Running the Program
After compiling, you can run the program using:

```bash
./aesdsocket
```

Ensure that you have the necessary permissions to write to `/var/tmp/aesdsocketdata`.

## Clean Up
To remove the compiled executable and any generated files, use the following command:

```bash
make clean
```

## Dependencies
- Standard C library
- POSIX socket API
- Syslog for logging messages

## Notes
Make sure to run the program with appropriate permissions, as it requires access to the `/var/tmp` directory for file operations.