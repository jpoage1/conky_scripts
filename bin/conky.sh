#!/bin/bash
configs=("~/.conkyrc" "~/.conky/pi.conf")

for config in "${configs[@]}"; do
	if ! pgrep -af "conky -c $config" >/dev/null; then
		conky -c "$config" &
	fi
done
