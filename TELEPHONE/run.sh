#!/bin/bash
pkill python

# cd to this script dir
# fixes activate venv from crontab
# shellcheck disable=SC2164
cd "${0%/*}"
# source venv/bin/activate

export DISPLAY=:0.0
xhost +

# For clean start
# Kill all relevant programs
sudo pkill -9 -f telephone.py
python3 src/telephone.py -c hh

