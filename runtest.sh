#!/bin/sh

GLIBC=/home/jun/eglibc-2.11.1-build

GCONV_PATH=${GLIBC}/iconvdata LC_ALL=C ${GLIBC}/elf/ld-linux.so.2 --library-path ${GLIBC}:${GLIBC}/math:${GLIBC}/elf:${GLIBC}/dlfcn:${GLIBC}/nss:${GLIBC}/nis:${GLIBC}/rt:${GLIBC}/resolv:${GLIBC}/crypt:${GLIBC}/nptl:${GLIBC}/dfp:/lib:/usr/lib "$@"
