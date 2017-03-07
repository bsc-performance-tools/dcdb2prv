#!/bin/sh

INPUT="$1"

while IFS='' read -r line || [[ -n "$line" ]]; do
	timestamp=$(echo "$line" | cut -d',' -f1)
	energy=$(echo "$line" | cut -d',' -f2)
	if [[ "$timestamp" != "Timestamp" ]]; then
	  echo "$NODE"-PWR,$(date +%s --date="$timestamp")000000000,"$energy"
	else
		NODE=$(echo "${energy}" | cut -d' ' -f1)
	fi
done < "$INPUT"

# vim: filetype=sh:tabstop=2:softtabstop=2:textwidth=80:colorcolumn=+0

