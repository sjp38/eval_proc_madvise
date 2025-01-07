#!/bin/bash

set -e

gcc -o eval_madv ./eval_madv.c
gcc -o eval_pmadv ./eval_pmadv.c

hint=4	#MADV_DONTNEED
nr_measures=100
sz_mem=$((256*1024*1024))	# 256 MiB

echo "# <nr_measures> 100"
echo "# <hint> <sz_mem> <sz_madv> <pmadv_batch> <madv|pmadv> <latency>"

for sz_madv_pg in 1 2 4 8 16 32 64 128 256 512 1024
do
	sz_madv=$((4096*sz_madv_pg))
	latency=$(./eval_madv "$hint" "$sz_mem" "$sz_madv" "$sz_p_madv" \
		"$nr_measures")
	pmadv_batch="n/a"
	echo "$hint $sz_mem $sz_madv $pmadv_batch madv $latency"

	for pmadv_batch in 1 2 4 8 16 32 64 128 256 512 1024
	do
		sz_p_madv=$((pmadv_batch * sz_madv))
		latency=$(./eval_pmadv "$hint" "$sz_mem" "$sz_madv" "$sz_p_madv" \
			"$nr_measures")
		echo "$hint $sz_mem $sz_madv $pmadv_batch pmadv $latency"

	done
done
