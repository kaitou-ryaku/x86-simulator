#!/bin/sh

TMPDIR="tmp"
mkdir -p $TMPDIR

if   echo $1 |egrep '\.c$'   > /dev/null ;then
  gcc -m32 -O0 -c $1 -o $TMPDIR/tmp.o
  objdump -d -M i386,intel $TMPDIR/tmp.o \
  |sed -e '1,6d' -e 's/ PTR / /g' -e 's/ <[^<> ]*>$//'

elif echo $1 |egrep '\.(asm|s)$' > /dev/null ;then
  nasm $1 -o $TMPDIR/tmp.o
  objdump -d -b binary -m i386 -M i386l,intel -D $TMPDIR/tmp.o \
  |sed -e '1,6d' -e 's/ PTR / /g' -e 's/ <[^<> ]*>$//'

elif echo $1 |egrep '\.(o|out|exe)$' > /dev/null ;then
  objdump -d -M i386,intel $1 \
  |sed -e '1,6d' -e 's/ PTR / /g' -e 's/ <[^<> ]*>$//'

elif echo $1 |egrep '\.bin$' > /dev/null ;then
  objdump -d -b binary -m i386 -M i386l,intel -D $1 \
  |sed -e '1,6d' -e 's/ PTR / /g' -e 's/ <[^<> ]*>$//'

fi
