#!/bin/sh
set -eu

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <firmware.bin|--bytes=N> <limit-bytes>" >&2
    exit 2
fi

case "$1" in
    --bytes=*) used_bytes=${1#--bytes=} ;;
    *) used_bytes=$(wc -c < "$1" | tr -d ' ') ;;
esac

limit_bytes=$2
case "$used_bytes:$limit_bytes" in
    *[!0-9:]*|:*|*:) echo "flash sizes must be positive byte counts" >&2; exit 2 ;;
esac

if [ "$used_bytes" -gt "$limit_bytes" ]; then
    echo "flash size $used_bytes bytes exceeds $limit_bytes-byte budget" >&2
    exit 1
fi

echo "flash size $used_bytes / $limit_bytes bytes"
