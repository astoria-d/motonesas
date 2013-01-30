#!/bin/bash

awk '
###ignore the line start with #
$0!~/#/ {
#second column addressing mode is indexed from 0.
    print "{ \""$1"\"" ", " $2-1 ", " "0x"$3 " }, "
}
' < 6502-opcode-table > opcode

