set terminal pdf
set output "cgpsSched.pdf"
set xlabel "Time (seconds)"
unset key
set ylabel "cgps CPU time (seconds)"
set style fill solid
set title "cgps CPU Usage"
plot "cgpsSchedAvg.txt" w boxes lt rgb "blue",\
     "cgpsFixDelayData.txt" w lines lw 0
