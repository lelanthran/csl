#!/bin/sh

gcc -dumpmachine | cut -f 1 -d .  | sed "s/-/_/g"

