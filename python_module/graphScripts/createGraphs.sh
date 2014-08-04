#!/bin/bash

# Use this file to create multiple graphs
# arg 1 = directoy of data files

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $1

# power usage
python $DIR/power.py *power.log

# cgps & gpsd sched switch
python $DIR/schedAvg.py *scheduler.log

# cgps fix delay
python $DIR/cgpsFixDelay.py

# modify delay plt file
echo `head -1 powerAvgInfoFormat.txt | awk '{ print $1 }'i` 0 > cgpsFixDelayData.txt
echo >> cgpsFixDelayData.txt
cp $DIR/delay.plt delay2.plt
for f in `ls cgpsFixDelayData* | sort`
do
    #echo \"$f\" w lines lw 5,\\ >> delay2.plt
    echo \"$f\" w filledcu,\\ >> delay2.plt
done
echo `tail -1 powerAvgInfoFormat.txt | awk '{ print $1 }'i` 0 >> cgpsFixDelayData.txt
cat delay2.plt | sed '$s/..$//' > delay3.plt

# user data
if [ -e *user.log ]
then
    cat *user.log | awk '{ if ($2 == '2') {print $1 " 0.0 0.0 2" }}' > userCustom.txt
    python $DIR/userGraph.py *user.log
    cp $DIR/user.plt user.plt
    gnuplot < user.plt
fi

# create graph & open
gnuplot < delay3.plt
gnuplot < $DIR/ra.plt
gnuplot < $DIR/gpsdSched.plt

rm *.plt *.txt
gnome-open ra.pdf
gnome-open delay.pdf
gnome-open gpsdSched.pdf
gnome-open user.pdf
#
