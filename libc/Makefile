
.PHONY: all clean install install-headers install-kernel

install: install-headers

install-headers:
	mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp -R include/. $(SYSROOT)$(INCLUDEDIR)/

clean:
