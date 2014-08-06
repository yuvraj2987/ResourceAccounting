set terminal pdf
set output "energy.pdf"
set key box opaque center top
set title "GPS Resource Accounting (model vs measurement)"
set xlabel "Time (seconds)"
set ylabel "Energy base on model (J)"
set y2label "Energy based on measurment (J)"
set ytics textcolor rgb "red"
set y2tics textcolor rgb "#2E8B57"
set style fill solid
set border lw 3
plot "energy_real.txt" using ($1):($2/1000000) title "Measured Energy" with lines lw 3 lt rgb "#2E8B57",\
     "energy_model.txt" using ($1):($2/1000000) title "Modeled Energy" with lines lw 3 lt rgb "red"
