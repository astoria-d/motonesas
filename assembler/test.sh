#!/bin/bash -x


#generate test instruction code
awk '
BEGIN {
line = 1
}
$0!~/#/ {
    if ($2 == 1) {
#zero page
        print $1 "\t$" line
    }
    else if ($2 == 2) {
#zero page, x
        print $1 "\t$" line ", x"
    }
    else if ($2 == 3) {
#zero page, y
        print $1 "\t$" line ", y"
    }
    else if ($2 == 4) {
#absolute
        print $1 "\t$10" line
    }
    else if ($2 == 5) {
#absolute, x
        print $1 "\t$10" line ", x"
    }
    else if ($2 == 6) {
#absolute, y
        print $1 "\t$10" line ", y"
    }
    else if ($2 == 7) {
#absolute indirect
        print $1 "\t($10" line ")"
    }
    else if ($2 == 8) {
#implied
        print $1
    }
    else if ($2 == 9) {
#accumulator
        print $1
    }
    else if ($2 == 10) {
#immediate
        print $1 "\t#$" line
    }
    else if ($2 == 11) {
#relative
        if ( line % 2 == 0)
            print $1 "\t+" line
        else
            print $1 "\t-" line
    }
    else if ($2 == 12) {
#indexed indirect
        print $1 "\t($" line ", X)"
    }
    else if ($2 == 13) {
#indirect indexed 
        print $1 "\t($" line "), Y"
    }
    line++
    if (line >= 100)
        line = 0
}
' < opcode-6502 > all-inst.asm

./motonesas all-inst.asm
./motonesas sample1.asm
./motonesas sample1-short.asm
./motonesas sample2.asm

cp sample*.o all-inst.o ../linker


