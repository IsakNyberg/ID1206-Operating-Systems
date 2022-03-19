set terminal png
set termoption enhanced

set output "yield_vs_noyield.png"

set title "Yielding vs not Yielding"

set key left top

#set logscale y 2

set ytics 1
#set xtics 1
#set grid mytics mytics
set grid

set xlabel "N"
set ylabel "Time taken to check primality of all numbers below N"

plot "yield.dat" u 1:2 title "With yield" with lines, "noyield.dat" u 1:2 title "Without yielding" with lines

