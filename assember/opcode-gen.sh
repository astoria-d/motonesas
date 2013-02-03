#!/bin/bash

##generate opcode table
awk '
###ignore the line start with #
$0!~/#/ {
#second column addressing mode is indexed from 0.
    print "{ \""$1"\"" ", " $2-1 ", " "0x"$3 " }, "
}
' < opcode-6502 > opcode

