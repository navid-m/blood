#!/bin/sh
./build.sh

case "$(uname -s)" in
Linux)
	sudo mv blood /usr/bin/blood
	;;
Darwin)
	sudo mv blood /usr/local/bin/blood
	;;
*)
	echo "error: unsupported host platform: $(uname -s)" >&2
	exit 1
	;;
esac
