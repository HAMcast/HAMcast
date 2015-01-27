HEVEA=hevea
HEVEAOPTS="png.hva"
HACHA=hacha
PDFLATEX=/usr/texbin/pdflatex
PDFOPTS=-output-directory=$(DIR) -aux-directory=$(DIR)
#document base name
DOC=hc_framework
DIR=./build
BASE=$(DIR)/$(DOC)

all: $(BASE).html $(BASE).pdf

#$(DIR)/index.html: $(BASE).html
#	$(HACHA) -tocter -o $(DIR)/index.html $(BASE).html

$(BASE).html: $(DOC).hva $(DOC).tex
	$(HEVEA) $(HEVEAOPTS) $(DOC).hva -o $(BASE).html $(DOC).tex
	$(HEVEA) $(HEVEAOPTS) $(DOC).hva -o $(BASE).html $(DOC).tex
	$(HEVEA) $(HEVEAOPTS) $(DOC).hva -o $(BASE).html $(DOC).tex

$(BASE).pdf: $(DOC).tex
	$(PDFLATEX) $(PDFOPTS) $(DOC).tex
	$(PDFLATEX) $(PDFOPTS) $(DOC).tex
	$(PDFLATEX) $(PDFOPTS) $(DOC).tex

partialclean:
	rm -f $(BASE).h{tml,aux,toc,ind} $(BASE).image.*

#all: $(BASE).html $(BASE).pdf

clean:
	rm -f $(DIR)/*

.PHONY: all clean
