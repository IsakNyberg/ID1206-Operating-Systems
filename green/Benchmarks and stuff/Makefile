#PDFLATEX = pdflatex
#GNUPLOT = gnuplot
#CC = gcc
#LATEXMK = latexmk
#all: report.pdf

#report.pdf: struct.c gnuplot/fragmentation.png

#benchmark.o: benchmark.c dlmall.c dlmall_without_merge.c
#	$(CC) -o benchmark.o benchmark2.c dlmall.c

#%.png: %.p %.dat
#	$(GNUPLOT) $<

all: report.pdf
	echo "done"

test.o: test.c green.c
	gcc -o test.o test.c green.c -Wno-deprecated-declarations -Wno-return-type


test2.o: test2.c green.c
	gcc -o test2.o test2.c green.c -Wno-deprecated-declarations -Wno-return-type

	
test3.o: test3.c green.c
	gcc -o test3.o test3.c green.c -Wno-deprecated-declarations -Wno-return-type
	
test3_pthread.o: test3_pthread.c
	gcc -o test3_pthread.o test3_pthread.c -lpthread -Wno-deprecated-declarations -Wno-return-type
	
pthread.dat: test3_pthread.o
	./test3_pthread.o > pthread.dat
	
green.dat: test3.o
	./test3.o > green.dat
	
green_vs_pthread.png: pthread.dat green.dat green_pthread.p
	gnuplot green_pthread.p	

	
test4_noyield.o: test4_noyield.c green.c
	gcc -o test4_noyield.o test4_noyield.c green.c -Wno-deprecated-declarations -Wno-return-type
	
test4_yield.o: test4_yield.c green.c
	gcc -o test4_yield.o test4_yield.c green.c -Wno-deprecated-declarations -Wno-return-type

yield.dat: test4_yield.o
	./test4_yield.o > yield.dat
	
noyield.dat: test4_noyield.o
	./test4_noyield.o > noyield.dat
	
yield_vs_noyield.png: yield.dat noyield.dat yield_noyield.p
	gnuplot yield_noyield.p

report.pdf: report.tex yield_vs_noyield.png green_vs_pthread.png
	pdflatex report.tex






