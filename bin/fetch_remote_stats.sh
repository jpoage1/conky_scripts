#!/bin/bash
ssh -o BatchMode=yes -o ConnectTimeout=3 conky@192.168.1.200 /home/conky/bin/conky.sh
