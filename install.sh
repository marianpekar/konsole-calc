#!/bin/sh

set -e

sudo gcc -O2 -Wall -Wextra -o konsole-calc src/main.c -lm

sudo install -Dm755 konsole-calc /usr/bin/konsole-calc
sudo install -Dm644 integration/konsole-calc.fish /usr/share/fish/vendor_conf.d/konsole-calc.fish
sudo rm -f build

echo "Done. Open a new fish session and try: 5+5 * 5"