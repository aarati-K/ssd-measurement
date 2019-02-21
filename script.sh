#!/bin/bash
for i in 1 2 4 8 16 32 64 128 256 512
do
	echo $i "kb"
	./c.out $i 0
	./r.out $i 0
	echo "--------------------"
done
