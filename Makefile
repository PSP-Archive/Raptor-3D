TARGET = lesson2
OBJS = main.o

CFLAGS = -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)
	
PSPBIN = $(PSPSDK)/../bin
CFLAGS += -I$(PSPSDK)/../include  -fsingle-precision-constant -g  
LIBS +=  -llua -lz  -lpspgu -ljpeg -lpng -lz -losl -lpspgu -lmikmod -lpspaudio -lGLU -lglut -lGLU -lGL -llualib -llua -lm -lc -lpsputility -lpspdebug -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpspvfpu -lpsplibc -lpspuser -lpspkernel -lpsprtc -lpsppower -lstdc++ -llua
LDFLAGS += -DMODULE_NAME="Raptor Alpha" psp-setup.c



EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Raptor Test
# PSP_EBOOT_ICON = hero.png
# PSP_EBOOT_PIC1 = bg.png


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak