documentation: whirl.ps

whirl.ps: whirl.dvi
	dvips -f whirl.dvi > whirl.ps

whirl.dvi: whirl.tex
	latex whirl.tex
	latex whirl.tex
	makeindex whirl
	latex whirl.tex
	latex whirl.tex
