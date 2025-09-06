#!/bin/sh
[ $# -eq 0 ] && { echo "Count: 0"; echo "Average: N/A"; exit 0; }
sum=0
for x in "$@"; do
  sum=$((sum + x))
done
echo "Count: $#"
echo "Average: $((sum / $#))"
