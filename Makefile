# Project name
NAME            = sound
# MSP430 MCU to compile for
CPU             = msp430g2553
# Optimisation level
OPTIMIZATION	= -O1

CFLAGS          = -mmcu=$(CPU) $(OPTIMIZATION) -Wall -g
SOURCES			= $(wildcard *.c)
OBJECTS			= $(SOURCES:.c=.o)
CC              = msp430-gcc

all: $(NAME).elf

# Build and link executable
$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Build object files from source files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Added to program with mspdebug
flash: $(NAME).elf
	mspdebug rf2500 "prog $(NAME).elf"

flasherase:
	mspdebug rf2500 erase

clean:
	rm -f $(NAME).elf $(OBJECTS)
