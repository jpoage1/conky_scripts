#!/bin/bash

export PROJECT_DIR="/srv/projects/telemetry"
export PATH="$PROJECT_DIR/bin:$PATH"
export SESSION="telemetry"

export YAZI_CONFIG_HOME="$PWD/.yazi"
# 1. Start a new session (detached)
tmux new-session -d -s "$SESSION" -n "dev" -c "$PROJECT_DIR"

tmux split-window -h -t telemetry:dev.0
tmux split-window -v -l 65% -t telemetry:dev.0

tmux set-environment -t telemetry PATH
tmux set-environment -t telemetry YAZI_CONFIG_HOME "$PWD/.yazi"-

tmux send-keys -t telemetry:0.0 "cd '$PROJECT_DIR'" C-m
tmux send-keys -t telemetry:0.0 'export YAZI_CONFIG_HOME="$PWD/.yazi"' C-m
tmux send-keys -t telemetry:0.0 "yazi" C-m
tmux select-pane -t telemetry:0.0 -T "NAVIGATOR"
tmux resize-pane -t telemetry:0.0 -y 40%

tmux send-keys -t telemetry:0.1 "cd '$PROJECT_DIR'" C-m
tmux send-keys -t telemetry:0.1 "clear; echo 'Loading nix-shell...'; nix-shell" C-m
tmux select-pane -t telemetry:0.1 -T "SHELL"

tmux send-keys -t telemetry:0.2 "cd '$PROJECT_DIR'" C-m
tmux send-keys -t telemetry:0.2 "vim" C-m
tmux select-pane -t telemetry:0.2 -T "EDITOR"
tmux resize-pane -t telemetry:0.2 -x 65%

tmux attach-session -t "$SESSION"
