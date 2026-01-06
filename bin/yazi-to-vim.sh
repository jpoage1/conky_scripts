#!/bin/bash
FILE_PATH="$1"

# Target the pane titled EDITOR
TARGET_ID=$(tmux list-panes -F'#D #T' -t telemetry | grep EDITOR | awk '{print $1}')

if [ -n "$TARGET_ID" ]; then
	# Send the file to the editor pane
	tmux send-keys -t "$TARGET_ID" Escape ":e $FILE_PATH" Enter
	# Focus the editor so you can start typing
	tmux select-pane -t "$TARGET_ID"
else
	# Fallback: if no EDITOR pane exists, open in the shell pane
	SHELL_ID=$(tmux list-panes -F "#{pane_id} #{pane_title}" | grep "SHELL" | awk '{print $1}')
	tmux send-keys -t "$SHELL_ID" "vim $FILE_PATH" Enter
fi
