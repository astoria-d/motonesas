#!/bin/bash

awk '{print "{ \""$1"\"" ", " $2 ", " "0x"$3 " }, "}' < 6502-opcode-table > opcode

