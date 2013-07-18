#!/bin/sh
find . -name "*.[chCHS]" -exec etags -a {} \;
cscope -b -R
