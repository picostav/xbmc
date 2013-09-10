include ../Makefile.include
DEPS= ../Makefile.include 0001-dontbuildswig.patch 0002-missing-libs.patch \
      0003-no-so-version.patch Makefile

# lib name, version
LIBNAME=libplist
VERSION=1.6
SOURCE=$(LIBNAME)-$(VERSION)
FILENAME=v$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

ICONV=$(PREFIX)/lib/libiconv.a
LIBDYLIB=$(PLATFORM)/build/src/libplist++.so

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); patch -p1 < ../0001-dontbuildswig.patch
	cd $(PLATFORM); sed -ie 's|TARGET_LINK_LIBRARIES( plist $${LIBXML2_LIBRARIES} |TARGET_LINK_LIBRARIES( plist m z $${LIBXML2_LIBRARIES} $(ICONV) |' src/CMakeLists.txt
	cd $(PLATFORM); rm -rf build; mkdir -p build
	cd $(PLATFORM)/build; CC=$(CC) CXX=$(CXX) cmake        \
	  -DCMAKE_C_FLAGS:STRING="$(CPPFLAGS) $(CFLAGS)"     \
	  -DCMAKE_CXX_FLAGS:STRING="$(CPPFLAGS) $(CXXFLAGS)" \
	  -DCMAKE_INCLUDE_PATH=$(PREFIX)/include             \
	  -DCMAKE_LIBRARY_PATH=$(PREFIX)/lib                 \
	  -DCMAKE_INSTALL_NAME_DIR=$(PREFIX)/lib             \
	  -DCMAKE_INSTALL_PREFIX=$(PREFIX)                   \
	  -DCMAKE_SYSTEM_NAME=Linux                          \
	  -DCMAKE_FIND_ROOT_PATH=$(platform_sdk_path) ..

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -j 1 -C $(PLATFORM)/build

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/build install
	rm -f $(PREFIX)/lib/libplist.la $(PREFIX)/lib/libplist.so $(PREFIX)/lib/libplist.so.1
	mv -f $(PREFIX)/lib/libplist.so.1.1.6 $(PREFIX)/lib/libplist.so
	$(RPL) -e "libplist.so.1" "libplist.so\x00\x00" $(PREFIX)/lib/libplist.so
	-$(READELF) --dynamic $(PREFIX)/lib/libplist.so | grep ibrary
	rm -f $(PREFIX)/lib/libplist++.la $(PREFIX)/lib/libplist++.so $(PREFIX)/lib/libplist++.so.1
	mv -f $(PREFIX)/lib/libplist++.so.1.1.6 $(PREFIX)/lib/libplist++.so
	$(RPL) -e "libplist++.so.1" "libplist++.so\x00\x00" $(PREFIX)/lib/libplist++.so
	$(RPL) -e "libplist.so.1" "libplist.so\x00\x00" $(PREFIX)/lib/libplist++.so
	-$(READELF) --dynamic $(PREFIX)/lib/libplist++.so | grep ibrary
	touch $@

clean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

