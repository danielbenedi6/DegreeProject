#!/bin/bash
COUNT=$2
OUTPUT=$1

echo "type,neurons,density,hits,building_time,search_time" > "${OUTPUT}.csv"
for file in ./tests/"${OUTPUT}"/*.rpl; do
  NEURONS=$(echo "$file" | cut -d'_' -f2)
  n=$(( $( od -vAn -N4 -tu4 < /dev/urandom ) % NEURONS ))
  c=$(( $( od -vAn -N4 -tu4 < /dev/urandom ) % 645 + 1 ))
  for i in $( seq "${COUNT}" ); do
    {
      echo -n "s,$( basename "${file}" | sed 's/^\(.*\)_\(.*\).rpl/\1,\2/' ),";
      ./bin/main -s tests/interneuron.CNG.swc "$file" $n $c;
      echo -n "p0,$( basename "${file}" | sed 's/^\(.*\)_\(.*\).rpl/\1,\2/' ),";
      ./bin/main -p0 tests/interneuron.CNG.swc "$file" $n $c;
    } >> "${OUTPUT}.csv"
  done
done