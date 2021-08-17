###############################################################################
# Makefile FAME emitter
#
# Oscar Orallo Pelaez
# Date: 17-10-2006
# Last update: 05-08-2007
###############################################################################

TARGET = fame$(target)
OBJS = src/fame$(target).o
CFLAGS = -O2 `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`
CC = gcc
STRIP = strip

# Default format ELF
ifeq ($(format),)
	format = elf
endif

ifeq ($(target),indepent)
fame.c : src/famec.c src/famec_opcodes.h
	cp src/famec.c fame.c
	cp src/famec_opcodes.h .

all: fame.c
else
$(TARGET) : $(OBJS)
	$(SH)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

all: $(TARGET)
endif



dist: $(TARGET)
	$(STRIP) $(TARGET)

uae: $(TARGET)
	./$(TARGET) --format=$(format) --inline-loop --accurate-timing --check-branches --multi-bank --undocumented --direct-mapping=16 --trace-mode --clock-counting --running-state
ifeq ($(target),sh4)
	@sed 's/io_cycle_counter/_io_cycle_counter/g' fame.s > fame.s.tmp
	@sed -e '25i\.global _m68kcontext' fame.s.tmp > fame.s.tmp2
	@sed -e '26i\.global _io_cycle_counter' fame.s.tmp2 > fame.s
	@rm -f fame.s.tmp fame.s.tmp2
else
ifeq ($(target),x86)
	@sed 's/io_cycle_counter/_io_cycle_counter/g' fame.asm > fame.asm.tmp
	@sed -e '29i\global _m68kcontext' fame.asm.tmp > fame.asm.tmp2
	@sed -e '30i\global _io_cycle_counter' fame.asm.tmp2 > fame.asm
	@rm -f fame.asm.tmp fame.asm.tmp2
endif
endif

dcastaway:
	./$(TARGET) --format=$(format) --inline-loop --accurate-timing --check-branches --undocumented --direct-mapping --trace-mode --clock-counting --irq-clocking
ifeq ($(target),sh4)
	@sed 's/io_cycle_counter/_io_cycle_counter/g' fame.s > fame.s.tmp
	@sed -e '25i\.global _m68kcontext' fame.s.tmp > fame.s.tmp2
	@sed -e '26i\.global _io_cycle_counter' fame.s.tmp2 > fame.s
	@rm -f fame.s.tmp fame.s.tmp2
else
ifeq ($(target),x86)
	@sed 's/io_cycle_counter/_io_cycle_counter/g' fame.asm > fame.asm.tmp
	@sed -e '29i\global _m68kcontext' fame.asm.tmp > fame.asm.tmp2
	@sed -e '30i\global _io_cycle_counter' fame.asm.tmp2 > fame.asm
	@rm -f fame.asm.tmp fame.asm.tmp2
endif
endif

neo: $(TARGET)
	./$(TARGET) --format=$(format) --inline-loop --running-state --direct-mapping
ifeq ($(target),sh4)
	@sed -e '25i\.global _m68kcontext' fame.s > fame.s.tmp
	@mv fame.s.tmp fame.s
else
ifeq ($(target),x86)
	@sed -e '29i\global _m68kcontext' fame.asm > fame.asm.tmp
	@mv fame.asm.tmp fame.asm
endif
endif

gplus: $(TARGET)
	./$(TARGET) --format=$(format) --inline-loop --running-state --accurate-timing --direct-mapping=16

gens: $(TARGET)
	./$(TARGET) --format=$(format) --inline-loop --running-state --clock-counting --irq-clocking --check-irqs --direct-mapping
ifeq ($(target),sh4)
	@sed -e '25i\.global _m68kcontext' fame.s > fame.s.tmp
	@mv fame.s.tmp fame.s
else
ifeq ($(target),x86)
	@sed -e '29i\global _m68kcontext' fame.asm > fame.asm.tmp
	@mv fame.asm.tmp fame.asm
endif
endif

clean:
ifeq ($(target),sh4)
	rm -f fame.s $(OBJS) $(TARGET)
else
ifeq ($(target),x86)
	rm -f fame.asm $(OBJS) $(TARGET)
endif
	rm -f fame.c famec_opcodes.h
endif
