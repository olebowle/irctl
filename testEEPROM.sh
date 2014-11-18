#!/bin/bash
macro_slots=8
macro_depth=8
wake_slots=8

echo "read out initially"
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -g /dev/hidraw0
	done
done
for w_slot in $(seq $wake_slots); do
	./irctl -dstm32 -w${w_slot} -g /dev/hidraw0
done

echo "write pattern to eeprom"
i=0
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -s$(printf '0x%02x%02x%02x%02x%02x%02x\n' $i $i $i $i $i $i) /dev/hidraw0
		((i++))
	done
done
for w_slot in $(seq $wake_slots); do
	./irctl -dstm32 -w${w_slot} -s$(printf '0x%02x%02x%02x%02x%02x%02x\n' $i $i $i $i $i $i) /dev/hidraw0
	((i++))
done

echo "read out pattern"
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -g /dev/hidraw0
	done
done
for w_slot in $(seq $wake_slots); do
	./irctl -dstm32 -w${w_slot} -g /dev/hidraw0
done

echo "reset eeprom"
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -r /dev/hidraw0
	done
done
for w_slot in $(seq $wake_slots); do
	./irctl -dstm32 -w${w_slot} -r /dev/hidraw0
done

echo "read out once more"
for m_slot in $(seq $macro_slots); do
	for i_slot in $(seq 0 $macro_depth); do
		./irctl -dstm32 -m${m_slot} -i${i_slot} -g /dev/hidraw0
	done
done
for w_slot in $(seq $wake_slots); do
	./irctl -dstm32 -w${w_slot} -g /dev/hidraw0
done
