#!/bin/bash

export PPATH="$( pwd -P )/tools/bin:$PATH"
export PPATH="$( pwd -P )/tools/i386-elf/bin:$PPATH"

/bin/bash --rcfile <(echo -e 'PATH="$PPATH"\nPS1="\[\033[01;37m\](\[\033[01;31m\]hydrax\[\033[01;37m\])\[\033[01;34m\] \w \$\[\033[00m\] "') -i



