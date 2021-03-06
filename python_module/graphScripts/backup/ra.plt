set terminal pdf
set output "ra.pdf"
set key box opaque center top
set title "GPS power and CPU Resource Accounting"
set xlabel "Time (seconds)"
set ylabel "Power (mW)"
set y2label "cgps CPU Usage"
set ytics textcolor rgb "#2E8B57"
set y2tics 0.02 nomirror tc lt 1
set ytics nomirror
set style fill solid
set border lw 3
set yrange [100:200]
plot "cgpsSchedAvg.txt" title "Sched Switch" axes x1y2 w boxes lt rgb "red",\
     "powerAvgInfoFormat.txt" using ($1):($2/1000) title "Power Avg" with lines lw 3 lt rgb "#2E8B57"
