#!/bin/bash

#create bash inis
MIDDLENAME=module
ARRAY_COUNTER=0
DURATION=100
START_BYTES=100
END_BYTES=1300
JUMP=200
INI_NAME=test_$MIDDLENAME"252"
INI_NAME_REMOTE=test_$MIDDLENAME"251"
HAMCAST_PATH=/users/localadmin/hamcast
HAMCAST="true"

PROGRAM_ARRAY=( #"/users/localadmin/hamcast/modules/mcpo/performance-ariba/build/performance-ariba"
		#"/users/localadmin/hamcast/modules/mcpo/performance-mcpo/build/testmcpo"
		"/users/localadmin/hamcast/modules/mcpo/performance-test/build/performancetest"
			)


X=$START_BYTES

	while [ $X -le $END_BYTES ]
	do
		echo "duration="$DURATION > $INI_NAME"_$X"
		echo "duration="$DURATION > $INI_NAME_REMOTE"_$X" 
		echo "ival=1" >> $INI_NAME"_$X"
		echo "ival=1" >> $INI_NAME_REMOTE"_$X"
		echo "program="${PROGRAM_ARRAY[$ARRAY_COUNTER]} >> $INI_NAME"_$X"
		echo "program="${PROGRAM_ARRAY[$ARRAY_COUNTER]} >> $INI_NAME_REMOTE"_$X"
		echo "arguments=-s -i 1 -p $X -t 90 -g ariba://myuri.de:1234" >> $INI_NAME"_$X"	#performancetest	
		#echo "arguments=-s -i 1 -p $X -n hamcast -t 110 -b \"hamcast{ip{141.22.26.251};tcp{5005}}\" -e \"ip{141.22.26.252};tcp{5005}\" -c node252" >> $INI_NAME"_$X"
		#echo "arguments=-s -p $X -t 90 -m ip" >> $INI_NAME"_$X"
		
		#echo "arguments=-r -i 1 -p $X -t 70 -g ariba://myuri.de:1234" >> $INI_NAME_REMOTE"_$X"	# performancetest
		echo "arguments=-r -i 1 -p $X -n hamcast -t 70 -b \"hamcast{ip{141.22.26.252};tcp{5005}}\" -e \"ip{141.22.26.251};tcp{5005}\" -c receiver" >> $INI_NAME_REMOTE"_$X"
		#echo "arguments=-r -i 1 -p $X -t 70 -m ip" >> $INI_NAME_REMOTE"_$X"
		
				

		if [ $HAMCAST == "true" ]
		then 
			echo "hamcast_path="$HAMCAST_PATH >> $INI_NAME"_$X"
			echo "hamcast_path="$HAMCAST_PATH >> $INI_NAME_REMOTE"_$X"
		fi
		
		X=$(($X+$JUMP))
	done


