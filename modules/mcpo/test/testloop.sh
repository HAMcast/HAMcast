#!/bin/bash



Y=100
while [ $Y -le 1300 ]
do
X=1
	while [ $X -le 50 ]
	do
	#./test.sh test_module252_$Y test_without_receive/module/b$Y
	#./test.sh test_ip252_$Y overnight-test/ip/b$Y
	#./test.sh test_module252_$Y overnight-test/module/b$Y
	#./test.sh test_module252_$Y check_for_packetloss/module-blub/b$Y
	#./test.sh test_ariba252_$Y test_new_ariba/b$Y
	./test.sh test_ariba252_$Y try_opt_level/b$Y
	X=$(($X+1))
	done
Y=$(($Y+100))
done

#Y=100
#while [ $Y -le 1300 ]
#do

#X=1
#		while [ $X -le 10 ]
#		do
		#echo test_ariba252_$Y.ini
		#./test.sh test_mcpo252_$Y overnight-test/mcpo/b$Y

		
#		X=$(($X+1))
		
#		done	
#Y=$(($Y+200))
#done


#Y=100
#while [ $Y -le 1300 ]
#do

#X=1
#		while [ $X -le 10 ]
#		do
		#echo test_ariba252_$Y.ini
		#./test.sh test_ariba252_$Y overnight-test/ariba/b$Yc

		
#		X=$(($X+1))
		
#		done	
#Y=$(($Y+200))
#done
