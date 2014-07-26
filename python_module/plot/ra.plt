set terminal pdf
set output "ra.pdf"
set key box opaque left
set title "GPS Resource Accounting Logs on Pi\nOutside (linpack running with cgps measurements)"
set xlabel "Time (seconds)"
set ylabel "Power"
plot "powerInfoFormat.txt" using 1:2 title "power" with lines,\
     "powerAvgInfoFormat.txt" using 1:2 title "powerAvg" with lines lw 5,\
     "socketInfoFormat.txt" using ($1):(100000):($3-$1):(100000) title "socket" with vectors filled nohead
#     "eventdeviceInfoFormat.txt" using ($2):(100000):(0):(100000) title "eventdevice" with vectors filled nohead
