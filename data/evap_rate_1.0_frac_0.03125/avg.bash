#!/bin/bash

sum1=0
sum2=0
sum3=0
sum4=0
count=0

for file in ./*.csv; do
    if [ -f "$file" ]; then
        lastline=$(tail -n 1 "$file")
        field1=$(echo "$lastline" | cut -d',' -f1)
        field2=$(echo "$lastline" | cut -d',' -f2)
        field3=$(echo "$lastline" | cut -d',' -f3)
        field4=$(echo "$lastline" | cut -d',' -f4)
        sum1=$(echo "scale=2; $sum1 + $field1" | bc)
        sum2=$(echo "scale=2; $sum2 + $field2" | bc)
        sum3=$(echo "scale=2; $sum3 + $field3" | bc)
        sum4=$(echo "scale=2; $sum4 + $field4" | bc)
        count=$((count+1))
    fi
done

if [ "$count" -gt 0 ]; then
    avg1=$(echo "scale=2; $sum1 / $count" | bc)
    avg2=$(echo "scale=2; $sum2 / $count" | bc)
    avg3=$(echo "scale=2; $sum3 / $count" | bc)
    avg4=$(echo "scale=2; $sum4 / $count" | bc)
    echo "Average of last row columns for all CSV files in directory ./data:"
    echo "Column 1: $avg1"
    echo "Column 2: $avg2"
    echo "Column 3: $avg3"
    echo "Column 4: $avg4"
else
    echo "No CSV files found in directory ./data"
fi

