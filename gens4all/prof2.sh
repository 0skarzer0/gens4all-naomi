#!/bin/bash
sh-elf-addr2line -s -p -f -i -e gens4allprof.elf < prof.txt | sed 'N;s/:[0-9]*//;P;D' | sort -n | uniq -c | sort -n -r | less
