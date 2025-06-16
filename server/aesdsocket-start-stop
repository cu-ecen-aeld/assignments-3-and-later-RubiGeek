#! /bin/sh

case "$1" in
    start)
        echo "Starting aesdsocket service..."
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
        echo "OK"
        ;;
    stop)
        echo "Stopping aesdsocket service..."
        # Command to stop the aesdsocket service
         start-stop-daemon -K -n aesdsocket 
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac

exit $?