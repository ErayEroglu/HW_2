all: advcalc2ir

advcalc2ir: main.c
	gcc main.c -o advcalc2ir
	