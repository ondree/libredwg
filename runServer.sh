#!/bin/bash

echo "Starting server for dwg translation"
python3 dwg_server.py -p $PORT 2>&1 1>&1
