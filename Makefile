BINARY = out

OBJECTS = \
	msx/Disk.o msx/AY8910.o msx/Debug.o msx/Floppy.o msx/I8251.o msx/I8255.o msx/MSX.o msx/Patch.o\
	msx/SCC.o msx/Sound.o msx/V9938.o msx/YM2413.o msx/Z80.o\
	psp/startup.o psp/pg.o psp/_clib.o\
	psp/menu.o psp/main_sound.o psp/main_sub.o psp/main_text.o psp/main_msx.o\
	psp/main.o\

DEFINES = -DBPP16 -DLSB_FIRST -DSOUND -DDISK

LIBRARY = psp/unziplib.a psp/lib_c.a

all: $(BINARY)

$(BINARY): $(OBJECTS)
	c:\psp\ps2dev\gcc\ee\bin\ee-ld -s -O0 $(OBJECTS) $(LIBRARY) -M -Ttext 8900000 -q -o $@ > main.map
	c:\psp\ps2dev\share\outpatch
	c:\psp\ps2dev\gcc\ee\bin\ee-strip outp
	c:\psp\ps2dev\share\elf2pbp outp "fMSX for PSP 0.61"

%.o : %.c
	c:\psp\ps2dev\gcc\ee\bin\ee-gcc -march=r4000 -O3 \
		-fomit-frame-pointer $(DEFINES) -g -mgp32 -mlong32 \
		-c $< -o $@

%.o : %.s
	c:\psp\ps2dev\gcc\ee\bin\ee-gcc -march=r4000 -g -mgp32 -c -xassembler -O -o $@ $<

clean:
	del /s /f *.o *.map
