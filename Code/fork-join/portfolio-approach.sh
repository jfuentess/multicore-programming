#!/bin/bash
./pqs $1 &
./pms $1 &
./ss $1 &
./pcs $1 $2 &
wait -n
kill 0
