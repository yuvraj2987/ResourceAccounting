set term pdf
set out 'delay.pdf'
set xlabel "Time (seconds)"
set title "GPS Fix Delay Pulse"
unset key
unset ytics
set ylabel "cgps instances"
set yrange [0:5]
set size 1,0.5
set style fill transparent pattern 4 border
plot "cgpsFixDelayData.txt" w lines lw 0,\
