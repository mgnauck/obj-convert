#!/bin/bash

# Usage: cat data.bin | bin2h.sh data > data.h

name=$(echo $1 | tr '[:lower:]' '[:upper:]')

echo "#ifndef" $name"_H"
echo "#define" $name"_H"
echo ""
echo "#include <stdint.h>"
echo ""
echo "const uint8_t $1[] = {"
cat - | xxd -i
echo "};"
echo ""
echo "#endif"
