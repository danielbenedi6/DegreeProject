#!/bin/bash
COUNT=$2
OUTPUT=$1
methods=("s" "p0" "p1" "p2" "p3")

echo "type,neurons,density,hits,building_time,search_time" > "${OUTPUT}.csv"
for file in ./tests/"${OUTPUT}"/*.rpl; do
  NEURONS=$(basename "${file}" | sed 's/0*\(.*\)_.*rpl/\1/' )
  n=$(( $( od -vAn -N4 -tu4 < /dev/urandom ) % NEURONS ))
  c=$(( $( od -vAn -N4 -tu4 < /dev/urandom ) % 645 ))
  for i in $( seq "${COUNT}" ); do
	for method in ${methods[@]}; do
		{
      		echo -n "${method},$( basename "${file}" | sed 's/^0*\(.*\)_\(.*\).rpl/\1,\2/' ),";
	      	TIME=$( timeout 60 ./bin/main -"${method}" tests/interneuron.CNG.swc "$file" "$n" "$c" )
		if [ $? -eq 0 ]; then
			echo "$TIME"
		else
			echo "-1,NaN,NaN"
		fi	
		} >> "${OUTPUT}.csv"
	done
  done
done
