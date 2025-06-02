#! /bin/sh

case "$1" in
    start)
        echo "Starting aesdsocket service..."
        aesdsocket -d
        echo "OK"
        ;;
    stop)
        echo "Stopping aesdsocket service..."
        # Command to stop the aesdsocket service
        aesdsocket -k
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac$(INSTALL) -m

exit $?