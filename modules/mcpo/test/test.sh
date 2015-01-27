#!/bin/bash
																														
#arguments: ini-file (if not standart "test.ini"), directory to push logs

LOGPATH=$PWD


trap "killall" 0

#FUNCTIONS
if [ -z "$1" ];
then
	INI=$PWD/test.ini
	echo "found no ini - using standart"
else 
	INI=$1
	echo "found ini - file" $INI
fi
if [ -z "$2" ];
then mkdir blub
else 
	mkdir -p $2
	LOGPATH=$PWD/$2
fi

#argument: ini-value
function read_ini() {
	
	echo `cat $INI| grep -v '^;'|grep $1 | awk -F"[=]" '{print $2}'`

}
function killall(){

kill $PID_APP
if $USE_MW;	then
	kill $PID_MW
fi
if [ $temp -le $end ]
	then	
	TIME_TO_SLEEP=$((($end-$temp)+1)) 
	echo "TIME_TO_SLEEP $TIME_TO_SLEEP"
	sleep $TIME_TO_SLEEP
	fi
exit 0
}



PROGRAM=`read_ini program`   
echo "found programm: " $PROGRAM

IVAL=`read_ini ival`
echo "found ival: " $IVAL

DURATION=`read_ini duration`
echo "found dur: " $DURATION


ARGUMENTS=`read_ini arguments`
echo "found arguments: " $ARGUMENTS


#ARGUMENTS="-i 1 -g ip://239.1.2.3:1234 -p 100 -s -t 20"

MIDDLEWARE=`read_ini hamcast_path`
echo "Found mw: " $MIDDLEWARE

MW_ARGS=`read_ini mw_args`
echo "Found mw args: " $MW_ARGS

CURRENT=$PWD




DATE=$(date +%y%m%d_%H%M)
WORKLOADOP="$LOGPATH/myexp_workload_$DATE.dat"
APPOUTPUT="$LOGPATH/myexp_app_$DATE.dat"
MW_OUTPUT="$LOGPATH/myexp_mw_$DATE.dat"

if [ -z "$MIDDLEWARE"  ] 
then
{
		echo "no middleware"
		USE_MW=false
	}
else
{
	cd $MIDDLEWARE/bin
	USE_MW=true
	CMD_MW="./middleware $MW_ARGS"
	nohup $CMD_MW > "$MW_OUTPUT" 2>&1 & 
	PID_MW=`ps aux | grep -v "grep" | grep -m 1 $CMD_MW | awk '{print$2}'`

	echo "middleware started: "$PID_MW
	cd $current
}
sleep 1
fi
CMD="$PROGRAM $ARGUMENTS"
echo "cmd: " $CMD

#nohup $CMD > $APPOUTPUT & #2>&1 < /dev/null &
nohup $CMD > "$APPOUTPUT"  2>&1 < /dev/null & 
#PID_APP="$!"
PID_APP=`ps aux | grep -v "grep" | grep -m 1 "$CMD" | awk '{print$2}'`
sleep 1 
echo "app pid: " $PID_APP

#echo "#pid_app: " $PID_APP


temp=`date +%s`
end=$[temp+DURATION ]
PATH_APP=/proc/$PID_APP
PATH_MW=/proc/$PID_MW

#   cputicksTOTAL	app{TOTALsize	rss	share	userticks kernelticks} mw {TOTALsize	rss	share	userticks kernelticks} mw{TOTALsize	rss	share	userticks kernelticks} mw {TOTALsize	rss	share	userticks kernelticks}
echo -e "#time\t\t#app-TOTALsize\t#app-rss\t#app-share\t#app-cpu-percent\t#mw_TOTALsize\t#mw_rss\t#mw_share\t#mw_cpu_percent" > $WORKLOADOP

#echo "#appoutput" >> $WORKLOADOP

typeset -i OLD_TOTAL_CPU=`cat /proc/uptime | cut -f1 -d " " | sed 's/\.//'`
OLD_APP_CPU=1
OLD_MW_CPU=1

while [ $temp -le $end ]  && [ -e $PATH_APP/statm ] 
do

	ram_app=$(awk '{print $1"\t""\t"$2"\t""\t"$3}' $PATH_APP/statm)
if $USE_MW;	then
	ram_mw=$(awk '{print $1"\t""\t"$2"\t"$3}' $PATH_MW/statm)

	else ram_mw="0\t0\t0\t"
fi

	IN=$(cat /proc/stat | grep "^cpu ")
	cpu=(`echo $IN |tr "," "\n"`)
	
	#IDLE=${cpu[4]}                        # Get the idle cpu time.
	DATE_CMD="date +%s%N"
	
	NOW=`$DATE_CMD`
	NOW=`expr $NOW / 1000`
	# Calculate the TOTAL cpu time.
#  	calc_total_cpu $cpu
	TOTAL_CPU=$TOTAL
	typeset -i TOTAL_CPU=`cat /proc/uptime | cut -f1 -d " " | sed 's/\.//'`
		
#	echo "stat: "$(cat $PATH_APP/stat)
	cpu_app=$(awk '{print $14"\t""\t"$15}' $PATH_APP/stat)
	let CPU_APP_SUM="$(awk '{print $14+$15}' $PATH_APP/stat)" 
#	echo "cpusum: "$CPU_APP_SUM
	
	let "DELTA_APP= $CPU_APP_SUM - $OLD_APP_CPU"
	let "DELTA_TOTAL=$TOTAL_CPU -$OLD_TOTAL_CPU"
#	echo "delta app: " $DELTA_APP " Delta TOTAL " $DELTA_TOTAL
	CPU_APP_PERCENT=$(echo "scale=2;(("$CPU_APP_SUM"-"$OLD_APP_CPU ")* 100)/("$TOTAL_CPU"-"$OLD_TOTAL_CPU")" | bc)
#	echo "TOTAL: "$TOTAL_CPU " app: "$CPU_APP_PERCENT " old app: "$OLD_APP_CPU " old_total_cpu " $OLD_TOTAL_CPU
	cpu_app=" "$CPUT_APP_PERCENT" "	
	
	OLD_APP_CPU=$CPU_APP_SUM
	
if $USE_MW;	then
	CPU_MW_SUM=$(awk '{print $14+$15}' $PATH_MW/stat)
	let "DELTA_MW= $CPU_MW_SUM - $OLD_MW_CPU"
	CPU_MW_PERCENT=$(echo "scale=2;(("$CPU_MW_SUM"-"$OLD_MW_CPU ")* 100)/("$TOTAL_CPU"-"$OLD_TOTAL_CPU")" | bc)	
	OLD_MW_CPU=$CPU_MW_SUM
	else
	cpu_mw="0"
fi

	echo -e "$NOW\t\t$ram_app\t\t$CPU_APP_PERCENT\t\t\t$ram_mw\t\t$CPU_MW_PERCENT" >> $WORKLOADOP
	#$TOTAL"\t"$ram_app"\t"$cpu_app"\t"$ram_mw"\t"$cpu_mw 
	

	OLD_TOTAL_CPU=$TOTAL_CPU

	sleep $IVAL
	temp=`date +%s`
done




