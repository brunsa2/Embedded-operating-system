EXEC = os
TESTEXEC = test-os

DEVICE = atmega328p
PROGRAMMER = avrispmkII
PROGRAMMER_PORT = usb
PROGRAMMED_DEVICE = m328p
CLOCK = 16000000
FUSES = -U lfuse:w:0xf7:m -U hfuse:w:0xd9:m -U efuse:w:0x07:m

SOURCE_DIRECTORY = src/
BUILD_DIRECTORY = bin/
TEST_DIRECTORY = test/

SOURCES := $(wildcard $(SOURCE_DIRECTORY)*.c)
TEST_SOURCES = $(wildcard $(TEST_DIRECTORY)*.c) $(SOURCES)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
TEST_OBJECTS = $(patsubst %.c,%.o,$(TEST_SOURCES))

CC = avr-gcc
CFLAGS = -g -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

TESTCC = gcc
TESTCFLAGS = -Wall -D__AVR_TestEnv__

AVRDUDE = avrdude -c $(PROGRAMMER) -P $(PROGRAMMER_PORT) -p $(PROGRAMMED_DEVICE)

.PHONY : all
all: $(BUILD_DIRECTORY)$(EXEC).hex

.PHONY : clean
clean:
	rm -rf $(BUILD_DIRECTORY)* $(OBJECTS) $(TEST_OBJECTS)

.PHONY : disasm
disasm: $(BUILD_DIRECTORY)$(EXEC).lss

%.lss: %.elf
	avr-objdump -d -S $< > $@

$(SOURCE_DIRECTORY)%.o: $(SOURCE_DIRECTORY)%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIRECTORY)$(EXEC).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILD_DIRECTORY)$(EXEC).elf $(OBJECTS)

$(BUILD_DIRECTORY)$(EXEC).hex: $(BUILD_DIRECTORY)$(EXEC).elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIRECTORY)$(EXEC).elf $(BUILD_DIRECTORY)$(EXEC).hex
	avr-size --format=avr --mcu=$(DEVICE) $(BUILD_DIRECTORY)$(EXEC).elf

.PHONY : connect
connect:
	$(AVRDUDE)

.PHONY : flash
flash: $(BUILD_DIRECTORY)$(EXEC).hex
	$(AVRDUDE) -U flash:w:$(BUILD_DIRECTORY)$(EXEC).hex:i

.PHONY : fuse
fuse:
	$(AVRDUDE) $(FUSES)

.PHONY : test
test: CC=$(TESTCC)
test: CFLAGS=$(TESTCFLAGS)
test: $(BUILD_DIRECTORY)$(TEXTEXEC)
	@bin/test-os

$(BUILD_DIRECTORY)$(TEXTEXEC): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $(BUILD_DIRECTORY)$(TESTEXEC) $(TEST_OBJECTS)
	
$(TEST_DIRECTORY)%.o: $(TEST_DIRECTORY)%.c
	$(TESTCC) $(TESTCFLAGS) -c $< -o $@