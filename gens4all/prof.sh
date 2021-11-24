#!/bin/bash
sh-elf-addr2line -s -p -f -e gens4all.elf < prof.txt | sed 'N;s/:[0-9]*//;P;D' | sort -n | uniq -c | sort -n -r | less
