SRCDIR = .

VPATH = $(SRCDIR)

include $(SRCDIR)/../Make.defaults

TOPDIR = $(SRCDIR)/..

CDIR=$(TOPDIR)/..

all:

clean:

install:
	mkdir -p $(INSTALLROOT)$(INCLUDEDIR)/efi
	mkdir -p $(INSTALLROOT)$(INCLUDEDIR)/efi/protocol
	mkdir -p $(INSTALLROOT)$(INCLUDEDIR)/efi/$(ARCH)
	$(INSTALL) -m 644 $(SRCDIR)/*.h $(INSTALLROOT)$(INCLUDEDIR)/efi
	$(INSTALL) -m 644 $(SRCDIR)/protocol/*.h $(INSTALLROOT)$(INCLUDEDIR)/efi/protocol
	$(INSTALL) -m 644 $(SRCDIR)/$(ARCH)/*.h $(INSTALLROOT)$(INCLUDEDIR)/efi/$(ARCH)
ifeq ($(ARCH),ia64)
	mkdir -p $(INSTALLROOT)$(INCLUDEDIR)/efi/protocol/ia64
	$(INSTALL) -m 644 $(SRCDIR)/protocol/ia64/*.h $(INSTALLROOT)$(INCLUDEDIR)/efi/protocol/ia64
endif

include $(SRCDIR)/../Make.rules
