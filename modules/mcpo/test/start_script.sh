#!/bin/bash



#TODO
#Programme anpassen			
# > in Schleife Argumente und aufrufe anpassen
#{s,r} tauschen bei jeweiligen Knoten
#

#arguments=-s -i 1 -p 100 -n hamcast -t 23 -b "hamcast{ip{141.22.26.251};tcp{5005}}" -e "ip{141.22.26.252}tcp{5005}" -c node1 -m ip

#PROGRAM_ARRAY=("/users/localadmin/hamcast/modules/mcpo/performance-ariba/build/performance-ariba"
#		"/users/localadmin/hamcast/modules/mcpo/performance-mcpo/build/mcpotest"
#		"/users/localadmin/hamcast/modules/mcpo/performance-test/build/performancetest")



PROGRAM_ARRAY=("/users/localadmin/hamcast/modules/mcpo/performance-ariba/build/performance-ariba"
		"/users/localadmin/hamcast/modules/mcpo/performance-mcpo/build/testmcpo"
		"/users/localadmin/hamcast/modules/mcpo/performance-test/build/performancetest")

PROGRAM_ARRAY_REMOTE=("/users/localadmin/hamcast/modules/mcpo/performance-ariba/build/performance-ariba"
		"/users/localadmin/hamcast/modules/mcpo/performance-mcpo/build/testmcpo"
		"/users/localadmin/hamcast/modules/mcpo/performance-test/build/performancetest")


INI_NAME="test.ini"
INI_NAME_REMOTE="test_remote.ini"
DATE=$(date +%y%m%d_%H%M)
DIR="test_"$DATE

SSH_ADDRESS="root@141.22.26.251"

DURATION=100 		
IVAL=1
BOOTSTRAP="\"hamcast{ip{141.22.26.251};tcp{5005}}\""
ENDPOINT="\"ip{141.22.26.252};tcp{5005}\""
#BOOTSTRAP="hamcast{ip{127.0.0.1};tcp{5005}}"
#ENDPOINT="ip{127.0.0.1}tcp{5004}"
ROLE_ARRAY=(r s)
NODE_NAME="node1"
PAYLOAD_ARRAY=(100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400)
LOOPS=2
HAMCAST_PATH="/users/localadmin/hamcast"
MIDDLEWARE_ARGS=""
GROUP_ADRESS="ariba://myuri.de:1234"

NODENAME_REMOTE="node2"
BOOTSTRAP_REMOTE="\"hamcast{ip{141.22.26.252};tcp{5005}}\""
ENDPOINT_REMOTE="\"ip{141.22.26.251};tcp{5005}\""
HAMCAST_PATH_REMOTE="/users/localadmin/hamcast"
MIDDLEWARE_ARGS_REMOTE=""
SSH_OUTPUT_PATH="/users/localadmin/hamcast/modules/mcpo/performance-ariba/test"



function create_test_ini() {
		echo "duration="$DURATION > $INI_NAME
		echo "duration="$DURATION > $INI_NAME_REMOTE
		echo "ival="$IVAL >> $INI_NAME
		echo "ival="$IVAL >> $INI_NAME_REMOTE
		echo "program="${PROGRAM_ARRAY[$1]} >> $INI_NAME
		echo "program="${PROGRAM_ARRAY_REMOTE[$1]} >> $INI_NAME_REMOTE
		if [ $1=="hamcast" ]
			echo "hamcast_path: "$HAMCAST_PATH
			then echo "hamcast_path="$HAMCAST_PATH >> $INI_NAME
			echo "hamcast_path="$HAMCAST_PATH_REMOTE>> $INI_NAME_REMOTE
		fi
}


LAST_REMOTE_CMD=""
function login_and_start(){
	sleep 5
	if [ -n "$LAST_REMOTE_CMD" ] 
		then
		ssh $SSH_ADDRESS "kill $LAST_REMOTE_CMD"
		ssh $SSH_ADDRESS "killall middleware"
	fi

	scp "./$INI_NAME_REMOTE" "$SSH_ADDRESS:$SSH_OUTPUT_PATH"
	SSH_CMD=$1
	SSH_OUTPUT_FILE=$2 
	#SSH_READY_CMD="nohup $CMD > "$SSH_OUTPUT_PATH/$SSH_OUTPUT_FILE"  2> "$SSH_OUTPUT_PATH/$SSH_OUTPUT_FILE.err"  &"
	SSH_READY_CMD="$CMD > "$SSH_OUTPUT_PATH/$SSH_OUTPUT_FILE"  2> "$SSH_OUTPUT_PATH/$SSH_OUTPUT_FILE.err"  &"	
	echo "SSH_READY_CMD: " $SSH_READY_CMD
	ssh $SSH_ADDRESS $SSH_READY_CMD
	LAST_REMOTE_CMD="$CMD"

}

function get_all_remote_outputs(){
	mkdir -p $PWD"/remote_ergebnisse"
	scp -r $SSH_ADDRESS":"$SSH_OUTPUT_PATH $PWD"/remote_ergebnisse"
	ssh $SSH_ADDRESS "kill $LAST_REMOTE_CMD"
	ssh $SSH_ADDRESS "killall middleware"
}

trap "get_all_remote_outputs" 0

for ROLE in ${ROLE_ARRAY[@]}
do
if [ $ROLE == "r" ]
then 
	DUR_PR=$(($DURATION-30))
	DUR_PR_REMOTE=$(($DURATION-10))
	ROLE_REMOTE="s"
else 
	DUR_PR=$(($DURATION-10))
	DUR_PR_REMOTE=$(($DURATION-30))
	ROLE_REMOTE="r"
fi
	for PAYLOAD  in ${PAYLOAD_ARRAY[@]}
	do
			
		
		X=1
		while [ $X -le $LOOPS ]
		do
			##########TEST 01############
			create_test_ini 0
			echo "arguments=-"$ROLE" -i "$IVAL" -p "$PAYLOAD" -n hamcast -t "$DUR_PR" -b "$BOOTSTRAP" -e "$ENDPOINT" -c "$NODE_NAME >> $INI_NAME 
			echo "arguments=-"$ROLE_REMOTE" -i "$IVAL" -p "$PAYLOAD" -n hamcast -t "$DUR_PR_REMOTE" -b "$BOOTSTRAP_REMOTE" -e "$ENDPOINT_REMOTE" -c "$NODE_NAME_REMOTE >> $INI_NAME_REMOTE
			#echo "arguments=-"$ROLE "-i "$IVAL "-p" $PAYLOAD" -n hamcast -t "$DUR_PR" -b "$BOOTSTRAP" -e "$ENDPOINT "-c "$NODE_NAME 
			
			
			if [ $ROLE_REMOTE == "r" ] 
			then			
			login_and_start $PROGRAM_ARRAY_REMOTE[0] "$DIR/ariba_R$ROLE_REMOTE_P"_"$PAYLOAD" 
			$PWD/test.sh $INI_NAME $DIR/"ariba_R"$ROLE"_P"$PAYLOAD

			else
			login_and_start $PROGRAM_ARRAY_REMOTE[0] "$DIR/ariba_R$ROLE_REMOTE"_P"$PAYLOAD"
			$PWD/test.sh $INI_NAME $DIR/"ariba_R"$ROLE"_P"$PAYLOAD
			fi

			##########TEST 02############
			create_test_ini 1
			echo "arguments=-"$ROLE" -i "$IVAL" -p "$PAYLOAD" -n hamcast -t "$DUR_PR" -b "$BOOTSTRAP" -e "$ENDPOINT" -c "$NODE_NAME >> $INI_NAME 
			echo " arguments=-"$ROLE_REMOTE" -i "$IVAL" -p "$PAYLOAD" -n hamcast -t "$DUR_PR_REMOTE" -b "$BOOTSTRAP_REMOTE" -e "$ENDPOINT_REMOTE" -c "$NODE_NAME_REMOTE >> $INI_NAME_REMOTE
			

			if [ $ROLE_REMOTE == "r" ] 
			then			
			login_and_start $PROGRAM_ARRAY_REMOTE[1] $DIR/"mcpo_R"$ROLE_REMOTE"_P"$PAYLOAD
			$PWD/test.sh ./$INI_NAME $DIR/"mcpo_R"$ROLE"_P"$PAYLOAD

			else
			login_and_start $PROGRAM_ARRAY_REMOTE[1] $DIR/"mcpo_R"$ROLE_REMOTE"_P"$PAYLOAD
			$PWD/test.sh ./$INI_NAME $DIR/"mcpo_R"$ROLE"_P"$PAYLOAD
			fi
#		
			
			
			##########TEST 03############
			create_test_ini 2 hamcast
			echo "arguments=-"$ROLE" -i "$IVAL" -p "$PAYLOAD" -t "$DUR_PR" -g "$GROUP_ADRESS >> $INI_NAME 
			echo "arguments=-"$ROLE_REMOTE" -i "$IVAL" -p "$PAYLOAD" -t "$DUR_PR_REMOTE" -g "$GROUP_ADRESS >> $INI_NAME_REMOTE

			
			if [ $ROLE_REMOTE == "r" ] 
			then			
			login_and_start $PROGRAM_ARRAY_REMOTE[2] $DIR/"module_R"$ROLE_REMOTE"_P"$PAYLOAD
			$PWD/test.sh ./$INI_NAME $DIR/"module_R"$ROLE"_P"$PAYLOAD

			else
			$PWD/test.sh ./$INI_NAME $DIR/"module_R"$ROLE"_P"$PAYLOAD
			login_and_start $PROGRAM_ARRAY_REMOTE[2] $DIR/"module_R"$ROLE_REMOTE"_P"$PAYLOAD
			fi
			killall middleware


			##########TEST 04############
			create_test_ini 0
			echo "arguments=-"$ROLE "-i "$IVAL" -p "$PAYLOAD" -t "$DUR_PR" -g " $GROUP_ADRESS" -m ip">> $INI_NAME 
			echo "arguments=-"$ROLE_REMOTE" -i "$IVAL" -p "$PAYLOAD" -t "$DUR_PR_REMOTE" -g "$GROUP_ADRESS" -m ip">> $INI_NAME_REMOTE
			
			if [ $ROLE_REMOTE == "r" ] 
			then			
			login_and_start $PROGRAM_ARRAY_REMOTE[2] $DIR/"ip_nativ_R"$ROLE_REMOTE"_P"$PAYLOAD
			$PWD/test.sh ./$INI_NAME $DIR/"ip_nativ_R"$ROLE"_P"$PAYLOAD

			else
			$PWD/test.sh ./$INI_NAME $DIR/"ip_nativ_R"$ROLE"_P"$PAYLOAD
			login_and_start $PROGRAM_ARRAY_REMOTE[2] $DIR/"ip_nativ_R"$ROLE_REMOTE"_P"$PAYLOAD
			fi

			

		X=$(($X+1))
		done
		
		
	done 
	
	get_all_remote_outputs

	

	
done
