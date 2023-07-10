import subprocess
from gc import collect
import json
import os
import shlex
from os.path import exists
import sys

cmd = ""
path = "/media/ubuntu/Data/saurabh/Simulator/gapbs/benchmark/"
if sys.argv[1] == "tc":
	cmd = f"./tc -f ${path}/roadU.sg"
if sys.argv[1] == "bc":
	cmd = f"./bc -f ${path}/roadU.sg"
if sys.argv[1] == "cc":
	cmd = f"./cc -f ${path}/roadU.sg"
if sys.argv[1] == "bfs":
	cmd = f"./bfs -f ${path}/roadU.sg"