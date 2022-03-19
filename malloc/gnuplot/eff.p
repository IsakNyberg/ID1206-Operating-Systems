set terminal png
set termoption enhanced

set output "efficiency.png"

set title ""

set key right top

set xlabel "Number of allocated blocks"
set ylabel "Percentage of useful allocated memory"

plot "eff_small_head.dat" u 1:2  title "Reduced head size" with lines, "eff_normal_head.dat" u 1:2  title "Normal head size" with lines
