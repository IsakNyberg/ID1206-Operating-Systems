set terminal png
set termoption enhanced

set output "green_vs_pthread.png"

set title "Green vs Pthread"

set key left top

set xlabel "Number of threads"
set ylabel "Seconds to execute task for all threads"

set ytics 0.5
set xtics 1
#set grid mytics mytics
set grid

plot "pthread.dat" u 1:2 title "Pthread" with lines, "green.dat" u 1:2 title "Green" with lines

