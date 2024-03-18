CC_X64	:= clang -target x86_64-w64-mingw32-gnu
CC_X86	:= clang -target i686-w64-mingw32-gnu
SOURCE	:= $(wildcard *.c)

HSHKEY	:= $(shell python3 -c "import random; print(hex(random.getrandbits(32)))")
ENCKEY	:= $(shell python3 -c "import random; print(hex(random.getrandbits(8)))")
CFLAGS	:= $(CFLAGS) -Os -fno-asynchronous-unwind-tables -nostdlib 
CFLAGS 	:= $(CFLAGS) -fno-ident -fpack-struct=8 -falign-functions=1
CFLAGS  := $(CFLAGS) -s -ffunction-sections -falign-jumps=1 -w
CFLAGS	:= $(CFLAGS) -falign-labels=1 -Wl,-TSectionLink.ld
CFLAGS	:= $(CFLAGS) -fdata-sections -fms-extensions -mno-sse
LFLAGS	:= $(LFLAGS) -Wl,-s,--no-seh,--enable-stdcall-fixup

OUTX64	:= grimreaper.x64.exe
OUTX86	:= grimreaper.x86.exe
SHLX64	:= grimreaper.x64.bin
SHLX86	:= grimreaper.x86.bin

REPLACE_FIX	      := $
REPLACE_OBF_HASH_MAKE := 's/OBF_HASH_MAKE(\([^)]*\))/\$(REPLACE_FIX){ obf_hash_make( \1 ) }/g'
REPLACE_OBF_STRA_MAKE := 's/OBF_STRA_MAKE(\([^)]*\))/\$(REPLACE_FIX){ obf_stra_make( \1 ) }/g'
REPLACE_OBF_STRW_MAKE := 's/OBF_STRW_MAKE(\([^)]*\))/\$(REPLACE_FIX){ obf_strw_make( \1 ) }/g'

all: $(SOURCE)
	@ $(CC_X64) asm/x64/Start.s bin/*.c crt/*.c asm/x64/GetIp.s -I. $(CFLAGS) $(LFLAGS) -o bin/$(OUTX64)
	@ $(CC_X86) asm/x86/Start.s bin/*.c crt/*.c asm/x86/GetIp.s -I. $(CFLAGS) $(LFLAGS) -o bin/$(OUTX86)
	@ python3 scripts/extract.py -f bin/$(OUTX64) -o $(SHLX64)
	@ python3 scripts/extract.py -f bin/$(OUTX86) -o $(SHLX86)

$(SOURCE):
	@ mkdir -p bin
	@ cp -rf $@ bin/$(basename $@).mako
	@ sed -i $(REPLACE_OBF_HASH_MAKE) bin/$(basename $@).mako
	@ sed -i $(REPLACE_OBF_STRA_MAKE) bin/$(basename $@).mako
	@ sed -i $(REPLACE_OBF_STRW_MAKE) bin/$(basename $@).mako
	@ python3 scripts/export_template.py -f bin/$(basename $@).mako -o bin/$(basename $@).c

clean:
	@ rm -rf bin
	@ rm -rf $(OUTX64) $(OUTX86)
	@ rm -rf $(SHLX64) $(SHLX86)

.PHONY: all $(SOURCE) clean
