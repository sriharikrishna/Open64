documentation: whirl.ps

clean:
	/bin/rm -f *.aux *.dvi *.idx *.ilg *.ind *.log *.toc

veryclean: clean
	/bin/rm -f *.ps

whirl.ps: whirl.dvi
	dvips -f whirl.dvi > whirl.ps

whirl.dvi: whirl.tex
	latex whirl.tex
	latex whirl.tex
	makeindex whirl
	latex whirl.tex
	latex whirl.tex
