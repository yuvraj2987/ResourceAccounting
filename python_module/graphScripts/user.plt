set terminal pdf
set output "user.pdf"
set yrange [0:2]
set title "User Status"
set xlabel "Time (seconds)"
set ylabel "Inside / Outside"
unset key
unset ytics
set yrange [0:1.2]
set size 1,0.6
plot "cgpsFixDelayData.txt" w lines lw 0,\
     "userInOut.txt" w lines lw 5 lt rgb "#2E8B57" title "Inside / Outside",\
     "userCustom.txt" using ($1):($2):($3):($4) title "User Marker" with vectors filled nohead lt rgb "#333333"
