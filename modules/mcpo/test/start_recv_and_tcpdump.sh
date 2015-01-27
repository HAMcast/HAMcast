#!/bin/bash
#arguments: $1 outputfile tcpdump 
trap "killall" 0

OUTP_TCP=$1
tcpdump  -i eth0 port 5005 -w $OUTP_TCP 2>&1 < /dev/null &

sleep 10


function killall(){
	sudo killall tcpdump
}
