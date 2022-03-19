set terminal png
set termoption enhanced

set output "fragmentation.png"

set title ""

set key right center

set xlabel "Number of allocated blocks"
set ylabel "Number of free blocks"

plot "fragment_with_merge.dat" u 1:2  title "with merge" with lines, "fragment_without_merge.dat" u 1:2 title "without merge" with lines
#, "fragment_small_head.dat" u 1:2 title "Smaller Head" with lines
# pt 2 ps 1
#pt 2 ps 1