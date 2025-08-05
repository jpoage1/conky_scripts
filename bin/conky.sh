#!/bin/bash

CONFIG_DIR="$HOME/.config/conky/conf-enabled"
configs=("$CONFIG_DIR"/*)

for config in "${configs[@]}"; do
	if [ -f "$config" ] && ! pgrep -af "conky -c $config" >/dev/null; then
		conky -c "$config" &
	fi
done
