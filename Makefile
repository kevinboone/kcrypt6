APP=kcrypt6
GUIAPP=kcrypt6_gui
APPNAME=$(APP)
 
UNAME := $(shell uname -o)

PLATFORM=unix
ifeq ($(UNAME),Msys)
	PLATFORM=win32
endif

ifeq ($(PLATFORM),win32)
  TARGETS=$(APP) $(GUIAPP) 
else
  TARGETS=$(APP) 
endif

all: $(TARGETS) 
include dependencies.mak

MAJOR_VERSION=6
MINOR_VERSION=0
VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

.SUFFIXES: .o .c

KLIB_OBJS=klib_error.o klib_string.o klib_getopt.o klib_getoptspec.o klib_object.o klib_wstring.o klib_log.o klib_buffer.o klib_list.o klib_convertutf.o klib_path.o 

CORE_OBJS=main.o kcrypt_ui.o mygetpass.o kcrypt_engine.o kcrypt_idea.o kcrypt_fast.o

UIOBJS=kcrypt_cmdui.o

ifeq ($(PLATFORM),win32)
  PLAT_OBJS=res.o 
  GUIOBJS=kcrypt_win32.o
  GUILDFLAGS=-mwindows
  BINDIR=c:\windows
  MANDIR=c:\windows
else
  PLAT_OBJS=
  GUILDFLAGS=
  BINDIR=/usr/bin
  MANDIR=/usr/share/man/man1
endif

OBJS=$(CORE_OBJS) $(KLIB_OBJS) $(PLAT_OBJS) 

CFLAGS=-Wall -O3 $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)

$(APP): $(OBJS) $(UIOBJS)
	$(CC) $(LDFLAGS) -s -o $(APP) $(OBJS) $(UIOBJS)

$(GUIAPP): $(OBJS) $(GUIOBJS)
	$(CC) $(LDFLAGS) $(GUILDFLAGS) -s -o $(GUIAPP) $(OBJS) $(GUIOBJS)

res.o: res.rc
	windres res.rc -o res.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -DMAJOR_VERSION=\"$(MAJOR_VERSION)\" -DMINOR_VERSION=\"$(MINOR_VERSION)\" -DVERSION=\"$(VERSION)\" -DAPPNAME=\"$(APPNAME)\" -c $*.c -o $*.o

clean:
	rm -f $(APP) $(APP).exe $(GUIAPP) $(GUIAPP).exe *.o dump *.stackdump 


srcdist: clean
	(cd ..; tar cvfz /tmp/$(APPNAME)-$(VERSION).tar.gz kcrypt6)


web: srcdist
	./makeman.pl > kcrypt6.man.html
	cp /tmp/$(APPNAME)-$(VERSION).tar.gz /home/kevin/docs/kzone5/target/
	cp kcrypt6.man.html /home/kevin/docs/kzone5/target/
	cp README_kcrypt6.html /home/kevin/docs/kzone5/source
	cp kcrypt6_faq.html /home/kevin/docs/kzone5/source



install: all
	cp -p kcrypt6 $(BINDIR)
	cp kcrypt6.1 $(MANDIR)	



