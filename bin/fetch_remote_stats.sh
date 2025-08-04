#!/bin/bash
ssh -o BatchMode=yes -o ConnectTimeout=3 jason@192.168.1.200 /home/jason/.conky/bin/conky.sh
