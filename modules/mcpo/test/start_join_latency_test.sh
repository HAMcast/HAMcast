#!/bin/bash



GROUP_AMOUNT=5
TEST_AMOUNT=5
REMOTE_USER=localadmin
PROG_RECV_TCPDUMP=/users/localadmin/hamcast/modules/mcpo/test/start_recv_and_tcpdump.sh

#Commands:
CMD_HC="/users/localadmin/hamcast/bin/middleware -i /users/localadmin/hamcast/bin/middleware.ini" 
CMD_LATENCY=/users/localadmin/hamcast/modules/mcpo/test_latencies/hamcast_latency/build/hamcast-latency


NODES=("141.22.28.31" "141.22.28.32")
#NODES=("node0" "node1" "node2" "node3" "node4" "node5" "node6" "node7" "node8" "node9")
# $1=ip of remote node, $2 count tests
function start_hc(){
	echo "starting hamcast at: " $1

	ssh "$REMOTE_USER"@"$1" "mkdir test_latency ; nohup $CMD_HC > /users/localadmin/test_latency/hamcast_node_$1_$2 2>&1 < /dev/null &"

	
}

# $1=ip of remote node, $2 count tests, $3 group to send to
function start_sender() {
	echo "starting sender at: " $1 " with group: "$3
	ssh "$REMOTE_USER"@"$1" "nohup $CMD_LATENCY -s -g $3 > test_latency/sender_node_$1_$2 2>&1 < /dev/null &"
	
}

# $1=ip of remote node, $2 count tests, $3 group to join
function start_receiver() {
	echo "starting receiver at: " $1 " with group: "$3
	ssh "$REMOTE_USER"@"$1" "nohup $CMD_LATENCY -g $3 > test_latency/receiver_node_$1_$2 2>&1 < /dev/null &"
	#ssh "$REMOTE_USER"@"$1" "nohup $PROG_RECV_TCPDUMP /users/localadmin/test_latency/tcpdump_node_$1_$2 2>&1 < /dev/null &"

}

# $1 number to create uri with like in test_latency
function create_uri(){
	URI="ariba://myuri$1.de:1234"	
}

function collecting_data(){
	
for s in ${NODES[*]}
	do
		echo "collecting data... from "$s
		CMD="scp localadmin@$s:~/test_latency/* ./"
		$CMD		
		ssh "$REMOTE_USER"@"$s" "killall middleware ; rm -r /tmp/hamcast; rm -r ~/test_latency"
		
	done
}


########## MAIN ########## MAIN ########## MAIN ########## MAIN ##########

for (( i=0; i<$TEST_AMOUNT; i++ ))
do
echo "ROUND: !"$i

##### start remote hamcast #####
	for n in ${NODES[*]}
	do
		start_hc $n $i
	done
	
##### start remote static mc-group ######
echo "Groups: " $GROUP_AMOUNT
j=0
for (( g=0; g<$GROUP_AMOUNT; g++ ))
do
		
		create_uri $j
		TO_SEND=$(($g+$i))
		echo "to send: "$TO_SEND 
		start_sender ${NODES[$TO_SEND]} $i $URI
		j=$(($j+1))
done


##### start remote receiver ######
j=0
for (( g=0; g<$GROUP_AMOUNT; g++ ))
do
		
		create_uri $j
		TO_RECEIVE=$(( ( $GROUP_AMOUNT+$g+($i%$GROUPS))%${#NODES[*]} )) 
		echo "to receive: " $TO_RECEIVE
		start_receiver ${NODES[$TO_RECEIVE]} $i $URI
		j=$(($j+1))
done
sleep 30 
collecting_data

	

	echo "fertig!"

done


