documentation: whirl.ps

whirl.ps: whirl.dvi
	dvi2ps -f whirl.dvi > whirl.ps

whirl.dvi: whirl.tex
	latex whirl.tex
	latex whirl.tex
	makeindex whirl.tex
	latex whirl.tex
	latex whirl.tex
