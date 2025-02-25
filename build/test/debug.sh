#!/bin/sh
kill $(pidof unit_target)
gdb -x init.gdb --args ./test -p -m "$@"
