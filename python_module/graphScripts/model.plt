set terminal pdf
set output "model.pdf"
set key box opaque center top
set title "GPS power Resource Accounting Model"
set xlabel "Time (seconds)"
set ylabel "Power (mW)"
set ytics textcolor rgb "#2E8B57"
set ytics nomirror
set style fill solid
set border lw 3
set yrange [100:200]
plot "powerModel.txt" using ($1):($2/1000) title "Power Model" with lines lw 3 lt rgb "#2E8B57"
