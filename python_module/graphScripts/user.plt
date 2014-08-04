set term pdf
set out 'user.pdf'
set yrange [0:2]
set title "GPS User Data"
set xlabel "Time (seconds)"
unset key
unset ytics
set size 1,0.5
plot "cgpsFixDelayData.txt" w lines lw 0,\
     "userInOut.txt" w lines lw 5 title "Inside / Outside",\
     "userCustom.txt" using ($1):($2):($3):($4) title "User Marker" with vectors filled nohead lt rgb "#333333"
