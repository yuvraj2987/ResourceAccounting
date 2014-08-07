set terminal pdf
set output "events.pdf"
set xlabel "Time (seconds)"
unset key
set ylabel "request-response events"
set style fill solid
set title "Request-Responses generated"
plot "events.txt" w boxes lt rgb "magenta",\
     "cgpsFixDelayData.txt" w lines lw 0
