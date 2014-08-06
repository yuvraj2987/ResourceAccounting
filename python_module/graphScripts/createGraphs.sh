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
    echo \"$f\" w lines lw 0.1,\\ >> delay2.plt
    #echo \"$f\" w filledcu,\\ >> delay2.plt
done
echo `tail -1 powerAvgInfoFormat.txt | awk '{ print $1 }'i` 0 >> cgpsFixDelayData.txt
cat delay2.plt | sed '$s/..$//' > delay3.plt

#power model
python $DIR/model.py

# user data
cat *user.log | awk '{ if ($2 == '2') {print $1 " 0.0 0.0 2" }}' > userCustom.txt
python $DIR/userGraph.py *user.log


echo ""
echo "energy consumed based on model (timestamp(S) + energy(uJ)): "
tail -1 energy_model.txt
echo ""
echo "energy consumed based on measurement (timestamp(S) + energy(uJ)): "
tail -1 energy_real.txt
echo ""

# create graph & open
gnuplot < delay3.plt
gnuplot < $DIR/power.plt
gnuplot < $DIR/energy.plt
gnuplot < $DIR/gpsdSched.plt
gnuplot < $DIR/cgpsSched.plt
gnuplot < $DIR/user.plt

rm *.plt *.txt
gnome-open delay.pdf
gnome-open power.pdf
gnome-open energy.pdf
gnome-open gpsdSched.pdf
gnome-open cgpsSched.pdf
gnome-open user.pdf
#
