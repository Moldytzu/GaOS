#!/bin/bash
# modified version of https://gist.github.com/mintsuki/2079737d76f30c7dae1e8cb791325082#file-qemu-prof-sh

set -e

ARCH="x86_64"
INTERVAL="0.001"
INITIALWAIT="0.1"
CMDLINE="-M q35 -cpu core2duo -m 2G -boot c -hda disk.hdd -no-reboot"
KERNEL="kernel/bin/kernel.elf"

SOCKET="$(mktemp)"
OUTFILE="$(mktemp)"
SUBSHELL="$(mktemp)"

trap "rm -f '$SOCKET' '$OUTFILE' '$SUBSHELL'" EXIT INT TERM HUP

(
echo $BASHPID >"$SUBSHELL"
sleep $INITIALWAIT
echo "profiling started"
while true; do
    echo "info registers" >>"$SOCKET"
    sleep $INTERVAL
done
) &

tail -f "$SOCKET" | qemu-system-"$ARCH" $CMDLINE -monitor stdio \
    | grep -o 'RIP=[0-9a-f]*' \
    | sed 's/RIP=//g' \
    | sort | uniq -c | sort -nr >"$OUTFILE"

kill $(cat "$SUBSHELL")

ISLINENUM=1
for p in $(cat "$OUTFILE"); do
    if [ $ISLINENUM = 1 ]; then
        ISLINENUM=0
        echo -ne "$p\t"
        continue
    fi

    if ! [ -z "$KERNEL" ]; then
        ./toolchain/x86_64/kernel/bin/x86_64-elf-addr2line -f -p -e "$KERNEL" "0x$p" | tr -d '\n'
        echo " (0x$p)"
    else
        echo "(0x$p)"
    fi

    ISLINENUM=1
done