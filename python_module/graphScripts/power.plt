set terminal pdf
set output "power.pdf"
set key box opaque center top
set title "GPS Resource Accounting (model vs measurement)"
set xlabel "Time (seconds)"
set ylabel "Power based on model (mW)"
set y2label "Power based on measurment (mW)"
set ytics textcolor rgb "red"
set y2tics textcolor rgb "#2E8B57"
set style fill solid
set border lw 3
set yrange [100:200]
set y2range [100:200]
plot "powerAvgInfoFormat.txt" using ($1):($2/1000) title "Measured Power" with lines lw 3 lt rgb "#2E8B57",\
     "powerModel.txt" using ($1):($2/1000) title "Modeled Power" with lines lw 3 lt rgb "red"
