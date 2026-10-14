#!/bin/bash

if [ $# -eq 0 ]; then
    echo "числа: $0 число1 число2 ..." >&2
    exit 1
fi

count=$#
sum=0

for num in "$@"; do
    sum=$((sum + num))
done

average=$(awk -v sum="$sum" -v cnt="$count" 'BEGIN { printf "%.3f", sum/cnt }')

echo "количество аргументов: $count"
echo "среднее арифметическое аргументов: $average"

