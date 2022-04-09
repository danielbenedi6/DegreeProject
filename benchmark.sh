#!/bin/bash
COUNT=$2
OUTPUT=$1

echo "type,filename,number_neurons,$( seq "$((COUNT-1))" | tr '\n' ',')$COUNT" > "$OUTPUT"

for file in ./tests/*.swc; do
  echo -n "s,${file},$( grep -c '^[[:blank:]]*[^[:blank:]#]' "${file}" )" >> "$OUTPUT"
  for i in $( seq "$COUNT" ) ; do
    echo -n ",$( ./bin/main -s "${file}" | sed 's/ us//' )" >> "$OUTPUT"
  done
  echo "" >> "$OUTPUT"
done

for file in ./tests/*.swc; do
  echo -n "p0,${file},$( grep -c '^[[:blank:]]*[^[:blank:]#]' "${file}" )" >> "$OUTPUT"
  for i in $( seq "$COUNT" ) ; do
    echo -n ",$( ./bin/main -p0 "${file}" | sed 's/ us//' )" >> "$OUTPUT"
  done
  echo "" >> "$OUTPUT"
done