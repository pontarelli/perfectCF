SRCDIR=src
OBJDIR=obj
CXX=g++
EXE= perfectCF
SRC=$(shell ls -R $(SRCDIR)/*.c*)
OBJ=$(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
OBJ_MAIN=$(EXE:%=$(OBJDIR)/%.o )
OBJ_COMMON=$(filter-out $(OBJ_MAIN),$(OBJ))

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

all: $(EXE)

$(EXE): $(OBJDIR) $(OBJ)
	@$(CXX) $(OBJ_COMMON) obj/$@.o -o $@    
	@echo -e '$(OK_COLOR)[*] Created executable  $@ $(NO_COLOR)'

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@    
	@echo "[*] Compiled" $<

clean:
	@rm -f $(EXE) $(OBJ)
	@echo "[*] Directory $(CURDIR) cleaned"

