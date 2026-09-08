#!/bin/env bash

function convert() {
    name=$1
    tail -n +2 $name.hex | sed -e 's/.*: //' -e 's/ /\n/g' | sed -e 's/\(..\)\(..\)\(..\)\(..\)/\4 \3 \2 \1/' | xxd -r -p > $name.bin
}

convert sum
convert mem
