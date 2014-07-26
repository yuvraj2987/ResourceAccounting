import os  
import time
import sys

start_time=time.time()

#fname = "../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S",time.localtime(start_time))+"_plot.log"

reference = "sched_plot_files"

#flog = open(fname, 'w')
fout = open('my_plot.plt', 'w')

#flog.write('set terminal png\nset output "my_plot.png"\nset key out vert center top\nset size 1, .95\nset title "schedtrace with cgps"\nset xlabel "Time (seconds)"\nunset ytics\n')
fout.write('set terminal png\nset output "my_plot.png"\nset key out vert center top\nset size 1, .95\nset title "schedtrace with cgps"\nset xlabel "Time (seconds)"\nunset ytics\n')

flag = True
s = ""
for root, dirs, filenames in os.walk(reference):
    for f in filenames:

        if flag:
            s += "plot "
            flag = False

        s += '"' + reference + '/' + f + '" using ($1):(100000):(0):(100000) title "' + f + '" with vectors filled nohead,\\\n'
        
#flog.write(s[:-3])
fout.write(s[:-3])
