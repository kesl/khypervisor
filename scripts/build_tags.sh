#!/bin/sh
find . ! \( \( -type d -path './*/guestos/linux' -o -path './*/u-boot*' -o -path './*/ucos-ii' \) -prune \) -name "*.[chCHS]" -exec etags -a {} \;
find . ! \( \( -type d -path './*/guestos/linux' -o -path './*/u-boot*' -o -path './*/guestos/ucos-ii' \) -prune \) -name "*.[chCHS]" -print >> ./cscope.files
cscope -b -q -k
rm cscope.files
