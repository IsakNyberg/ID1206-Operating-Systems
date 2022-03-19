set terminal png
set termoption enhanced

set output "find_call.png"

set title ""

set key right top

set xlabel "Number of allocated blocks"
set ylabel "Block size comparisons per malloc"

plot "find_call_lists.dat" u 1:2  title "With separate lists" with lines, "find_call_normal.dat" u 1:2  title "Single free list" with lines
