MODULE_TOPDIR = ../..

include $(MODULE_TOPDIR)/include/Make/Other.make

TARGETS := $(patsubst %.py,$(TOOLSDIR)/%.py,html.py groff.py g.html2man.py)

default: $(TARGETS)

$(TOOLSDIR)/g.html2man.py: g.html2man.py
	$(INSTALL) $< $@

$(TOOLSDIR)/%.py: %.py
	$(INSTALL_DATA) $< $@
