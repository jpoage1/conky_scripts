#!/bin/bash
# Map mount points and devices
mounts=(
  "/dev/bork/nixos"
  "/dev/bork/nix-config"
  "/dev/system/home"
  "/dev/bork/home"
  "/dev/bork/bulk"
  "/dev/bork/projects"
  "/dev/bork/git"
  "/dev/bork/shared"
  "/dev/bork/backup"
  "/dev/bork/docker"
)

printf "%-25s %-8s %-8s %-8s %-10s %-10s\n" "Mount Point" "Used" "Size" "Use%" "Read/s" "Write/s"

for entry in "${mounts[@]}"; do
  dev="${entry%%:*}"
  mp="${entry#*:}"
  # df output for device
  read -r used size usep < <(df -h "$dev" | awk 'NR==2 {print $3, $2, $5}')
  # disk I/O stats: adapt device name to block device, e.g. sda, nvme0n1p1, etc.
  blkdev=$(lsblk -no pkname "$dev" 2>/dev/null || echo "")
  [[ -z $blkdev ]] && blkdev=$(basename "$dev")
  # read/write sectors from /sys/block
  read_sectors=$(awk '{print $3}' /sys/block/$blkdev/stat 2>/dev/null)
  write_sectors=$(awk '{print $7}' /sys/block/$blkdev/stat 2>/dev/null)
  # convert sectors to KB (512 bytes per sector assumed)
  read_kb=$((read_sectors * 512 / 1024))
  write_kb=$((write_sectors * 512 / 1024))
  printf "%-25s %-8s %-8s %-8s %-10s %-10s\n" "$mp" "$used" "$size" "$usep" "${read_kb}KB" "${write_kb}KB"
done
