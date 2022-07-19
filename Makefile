# based on makefile in:
# https://github.com/suloku/gcmm/

default: gc

run: rungc

clean: gc-clean wii-clean

wii:
	$(MAKE) -f Makefile.wii 

gc:
	echo $(MAKE) -f Makefile.gc
	$(MAKE) -f Makefile.gc 

runwii:
	$(MAKE) -f Makefile.wii run

rungc:
	$(MAKE) -f Makefile.gc run

reloadgc:
	$(MAKE) -f Makefile.gc reload

gc-clean:
	$(MAKE) -f Makefile.gc clean

wii-clean:
	$(MAKE) -f Makefile.wii clean

