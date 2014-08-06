set terminal pdf
set output "gpsdSched.pdf"
set xlabel "Time (seconds)"
unset key
set ylabel "gpsd CPU time (seconds)"
set style fill solid
set title "gpsd CPU Usage"
plot "gpsdSchedAvg.txt" w boxes lt rgb "green",\
     "cgpsFixDelayData.txt" w lines lw 0
