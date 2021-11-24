#!/bin/bash
sh-elf-addr2line -s -p -f -e gens4all.elf < prof.txt | sort -n | uniq -c | sort -n -r | less
