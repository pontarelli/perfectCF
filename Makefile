SRCDIR=src
OBJDIR=obj
CXX=g++
EXE= perfectCF
EXE2= LPMstat
SRC=$(shell ls -R $(SRCDIR)/*.c*)
OBJ=$(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

MD5VAL="$(shell md5sum src/*.cpp)"
COMPILE_TIME_VAL=$(shell date)

CFLAGS=-Wall -O2  -fpermissive -Wuninitialized -std=c++0x -m64  -Wall -D'COMPILE_TIME="$(COMPILE_TIME_VAL)"' -D'MD5=$(MD5VAL)'
HAVE_SSE=$(filter-out 0,$(shell grep sse4_2 /proc/cpuinfo | wc -l))
CFLAGS+=$(if $(HAVE_SSE),-msse4.2)

NO_COLOR=\x1b[0m
OK_COLOR=\x1b[32;01m
ERROR_COLOR=\x1b[31;01m
WARN_COLOR=\x1b[33;01m
OK_STRING=$(OK_COLOR)[OK]$(NO_COLOR)

all: $(EXE) $(EXE2)

$(EXE): $(OBJDIR) $(OBJ)
	@g++ obj/CF.o obj/city.o obj/crc.o obj/utils.o obj/xxhash.o obj/perfectCF.o -o perfectCF    
	@echo -e '$(OK_COLOR)[*] Created executable  $@ $(NO_COLOR)'

$(EXE2): $(OBJDIR) $(OBJ)
	@g++ obj/LPMstat.o -o  $@    
	@echo -e '$(OK_COLOR)[*] Created executable  $@ $(NO_COLOR)'

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@    
	@echo "[*] Compiled" $<

clean:
	@rm -f $(EXE) $(EXE2) $(OBJ)
	@echo "[*] Directory $(CURDIR) cleaned"

