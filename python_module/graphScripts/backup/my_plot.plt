set terminal png
set output "my_plot.png"
set key out vert center top
set size 1, .95
set title "schedtrace with cgps"
set xlabel "Time (seconds)"
unset ytics
plot "sched_plot_files/cgps-2626" using ($1):(100000):(0):(100000) title "cgps-2626" with vectors filled nohead