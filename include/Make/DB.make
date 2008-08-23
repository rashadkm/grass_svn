
# common dependencies and rules for building DBMI driver

PACKAGE = "grassmods"

include $(MODULE_TOPDIR)/include/Make/Vars.make
include $(MODULE_TOPDIR)/include/Make/Rules.make
include $(MODULE_TOPDIR)/include/Make/Html.make
include $(MODULE_TOPDIR)/include/Make/Compile.make

EXTRA_INC += -I$(MODULE_TOPDIR)/lib/db/dbmi_driver

dbmi: $(DBDRIVERDIR)/$(PGM)$(EXE) db_html

db_html: $(HTMLDIR)/grass-$(PGM).html

$(DBDRIVERDIR)/$(PGM)$(EXE): $(ARCH_OBJS) $(DEPENDENCIES)
	$(call linker)

.PHONY: dbmi db_html
