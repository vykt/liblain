#!/bin/sh
gdb -x init.gdb --args ./test "$@"
