SUBDIRS = utils interfaces app data pics
EXTRA_DIST = AUTHORS COPYING.LIB ChangeLog DESIGN NEWS README TODO

messages:
	rm -f tips.cpp
	(cd data && $(PREPARETIPS) > ../tips.cpp)
	$(EXTRACTRC) ./*/*.rc >> ./rc.cpp
	$(XGETTEXT) `find . -name "*.cpp"` -o $(podir)/kate.pot
	rm -f tips.cpp

include ../admin/Doxyfile.am

