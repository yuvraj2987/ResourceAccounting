set terminal pdf
set output "gpsdSched.pdf"
set xlabel "Time (seconds)"
unset key
set ylabel "gpsd CPU Usage"
set style fill solid
set title "gpsd Sched Switch"
plot "gpsdSchedAvg.txt" title "gpsd Sched Switch" w boxes,\
     "cgpsFixDelayData.txt" w lines lw 0
