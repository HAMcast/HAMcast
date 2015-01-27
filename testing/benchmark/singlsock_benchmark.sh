#!/usr/bin/env bash
#Last modified: 23/05/11 14:35:54(CET) by Sebastian Meiling

BASEDIR=$(pwd)
DATE=$(date +%y%m%d)
#sender
S_SSH_IP="141.22.26.252"
S_SSH_USER="root"
S_MIDDLEWARE_DIR="$BASEDIR/hamcast/middleware"
S_LOGDIR="$BASEDIR/$DATE.logs"
S_BENCHMARK_DIR="$BASEDIR/hamcast/testing/benchmark"
S_LIBHAMCAST_DIR="$BASEDIR/hamcast/libhamcast/.libs"
S_DEV="eth1"

#empfaenger
R_SSH_IP="141.22.26.237"
R_SSH_USER="$S_SSH_USER"
R_MIDDLEWARE_DIR="$S_MIDDLEWARE_DIR"
R_LOGDIR="$S_LOGDIR" 
R_BENCHMARK_DIR="$S_BENCHMARK_DIR"
R_LIBHAMCAST_DIR="$S_LIBHAMCAST_DIR"
R_DEV="eth0"

#benchmark settings
R_DURATION=40 #wie lange empfangen werden soll, in sekunden
S_DURATION=41 #wie lange gesendet werden soll, in sekunden
PAYLOAD=(100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400)
MODE=(n h o) #h=hamcast, n=native, o=overlay
LOOPS=25 #anzahl der durchläufe
L_LOGDIR="$BASEDIR/$DATE.all" #lokales verzeichniss wo logs hinkopiert werden nach dem benchmark

_ssh_exec(){
    ssh -fn "${1}@${2}" "$3"
}

_s_ssh_exec(){
    _ssh_exec "$S_SSH_USER" "$S_SSH_IP" "$@"
}

_r_ssh_exec(){
    _ssh_exec "$R_SSH_USER" "$R_SSH_IP" "$@"
}

_restart_middleware(){
    [[ "$1" == "s" ]] && {
        echo "restarting sender middleware..."
        local MIDDLEWARE_DIR="$S_MIDDLEWARE_DIR"
        local LIBHAMCAST_DIR="$S_LIBHAMCAST_DIR"
        local USER="$S_SSH_USER"
        local HOST="$S_SSH_IP"
    }
    [[ "$1" == "r" ]] && {
        echo "restarting receiver middleware..."
        local MIDDLEWARE_DIR="$R_MIDDLEWARE_DIR"
        local LIBHAMCAST_DIR="$R_LIBHAMCAST_DIR"
        local USER="$R_SSH_USER"
        local HOST="$R_SSH_IP"
    }
    _ssh_exec "$USER" "$HOST" "killall middleware &>/dev/null"
    sleep 2
    _ssh_exec "$USER" "$HOST" "cd $MIDDLEWARE_DIR; LD_LIBRARY_PATH=$LIBHAMCAST_DIR nohup $MIDDLEWARE_DIR/middleware > mw.out 2>&1 < /dev/null &"
    sleep 2
    _ssh_exec "$USER" "$HOST" 'renice -n -5 -p `pgrep middleware &>/dev/null`'
}

_benchmark(){
    #default route setzen, aonsten weiß der kernel unter umständen nicht wohin er die mcast pakete senden soll
    echo "setting sender default route to $S_DEV"
    _s_ssh_exec "sudo route add default dev $S_DEV &>/dev/null"
    echo "setting receiver default route to $R_DEV"
    _r_ssh_exec "sudo route add default dev $R_DEV &>/dev/null"

    echo "creating receiver logdir: $R_LOGDIR"
    _r_ssh_exec "mkdir -p $R_LOGDIR"
    echo "creating sender logdir $S_LOGDIR"
    _s_ssh_exec "mkdir -p $S_LOGDIR"

    echo "creating local logdir $L_LOGDIR"
    mkdir -p "$L_LOGDIR"

    i=0 #ansonsten ist i im schleifenrumpf nicht verfuegbar
    for (( i = 0 ; i < $LOOPS ; i++ )); do
        for (( p = 0 ; p < ${#PAYLOAD[@]} ; p++ )); do
            for m in ${MODE[@]}; do
                echo -n "PAYLOAD: ${PAYLOAD[$p]}, SEND_DURATION: $S_DURATION,  RECV_DURATION: $R_DURATION, RUN: ${i}, "
                MODUS=$m
                if [[ "$m" = "h" ]]; then
                    echo " Mode: HAMCAST"
                    MODUS="h"
                    _ssh_exec "$R_SSH_USER" "$R_SSH_IP" "cd $R_MIDDLEWARE_DIR; cp middleware.ini.ip middleware.ini"
                    _restart_middleware "r"
                    _ssh_exec "$S_SSH_USER" "$S_SSH_IP" "cd $S_MIDDLEWARE_DIR; cp middleware.ini.ip middleware.ini"
                    _restart_middleware "s"
                    local S_LOGFILE="send_hamcast"
                    local R_LOGFILE="recv_hamcast"
                fi
                if [[ "$m" = "o" ]]; then
                    echo " Mode: Scribe"
                    MODUS="h"
                    _ssh_exec "$R_SSH_USER" "$R_SSH_IP" "cd $R_MIDDLEWARE_DIR; cp middleware.ini.scribe middleware.ini"
                    _restart_middleware "r"
                    _ssh_exec "$S_SSH_USER" "$S_SSH_IP" "cd $S_MIDDLEWARE_DIR; cp middleware.ini.scribe middleware.ini"
                    _restart_middleware "s"
                    local S_LOGFILE="send_scribe"
                    local R_LOGFILE="recv_scribe"
                fi
                if [[ "$m" = "n" ]]; then
                    echo " Mode: NATIVE"
                    MODUS="n"
                    local S_LOGFILE="send_native"
                    local R_LOGFILE="recv_native"
                fi

                local S_LOGFILE=${S_LOGFILE}_${PAYLOAD[$p]}_${i}.log
                local R_LOGFILE=${R_LOGFILE}_${PAYLOAD[$p]}_${i}.log
                echo "Starting receiver..."
                ssh "$R_SSH_USER"@"$R_SSH_IP" "LD_LIBRARY_PATH=$R_LIBHAMCAST_DIR nohup $R_BENCHMARK_DIR/main -$MODUS -b ${PAYLOAD[$p]} -d $R_DURATION -r > $R_LOGDIR/$R_LOGFILE 2>&1 < /dev/null &"
                echo "Starting sender..."
                sleep 2
                ssh "$S_SSH_USER"@"$S_SSH_IP" "LD_LIBRARY_PATH=$S_LIBHAMCAST_DIR $S_BENCHMARK_DIR/main -$MODUS -b ${PAYLOAD[$p]} -d $S_DURATION -s > $S_LOGDIR/$S_LOGFILE"
                echo "sender finished"
                sleep 2
                echo "killing sender middleware"
                _s_ssh_exec "killall middleware"
                echo "killing receiver middleware"
                _r_ssh_exec "killall middleware"
                sleep 2
            done
            echo -e "----------------\n\n\n"
        done
    done
    scp -r ${S_SSH_USER}@${S_SSH_IP}:${S_LOGDIR}/* ${L_LOGDIR}/
    scp -r ${R_SSH_USER}@${R_SSH_IP}:${R_LOGDIR}/* ${L_LOGDIR}/
}

_benchmark
