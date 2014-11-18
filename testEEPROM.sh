#!/bin/bash
macro_slots=8
macro_depth=8
i=0
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -s$(printf '0x%02x%02x%02x%02x%02x%02x\n' $i $i $i $i $i $i) /dev/hidraw0
		((i++))
	done
done

i=0
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -g /dev/hidraw0
		((i++))
	done
done
