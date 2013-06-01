PROGRAMMER = avrispmkII
PROGRAMMER_PORT = usb
PROGRAMMED_DEVICE = m328p
CLOCK = 16000000
DEVICE = atmega328p
FUSES = -U lfuse:w:0xf7:m -U hfuse:w:0xd9:m -U efuse:w:0x07:m

SOURCE_DIRECTORY = src/
BUILD_DIRECTORY = bin/

SOURCES := $(wildcard $(SOURCE_DIRECTORY)*.c) $(wildcard $(SOURCE_DIRECTORY)network/*.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

AVRDUDE = avrdude -c $(PROGRAMMER) -P $(PROGRAMMER_PORT) -p $(PROGRAMMED_DEVICE)
COMPILE = avr-gcc -g -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

all: $(BUILD_DIRECTORY)main.hex

clean:
	rm -rf $(BUILD_DIRECTORY)* $(OBJECTS)
    
disasm: $(BUILD_DIRECTORY)main.lss

%.lss: %.elf
	avr-objdump -d -S $< > $@

$(SOURCE_DIRECTORY)%.o: $(SOURCE_DIRECTORY)%.c 
	$(COMPILE) -c $< -o $@

$(BUILD_DIRECTORY)main.elf: $(OBJECTS)
	$(COMPILE) -o $(BUILD_DIRECTORY)main.elf $(OBJECTS)

$(BUILD_DIRECTORY)main.hex: $(BUILD_DIRECTORY)main.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIRECTORY)main.elf $(BUILD_DIRECTORY)main.hex
	avr-size --format=avr --mcu=$(DEVICE) $(BUILD_DIRECTORY)main.elf

connect:
	$(AVRDUDE)

flash: $(BUILD_DIRECTORY)main.hex
	$(AVRDUDE) -U flash:w:$(BUILD_DIRECTORY)main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)
