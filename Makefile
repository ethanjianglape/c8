CC = g++
CFLAGS = -std=c++20 -O3 -Wall -Wextra
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system

c8: main.o cpu.o memory.o opcodes.o vga.o ui.o
	$(CC) $(CFLAGS) -o bin/c8 bin/main.o bin/cpu.o bin/memory.o bin/opcodes.o bin/vga.o bin/ui.o $(LDLIBS)

clean:
	rm bin/main.o bin/cpu.o bin/memory.o bin/opcodes.o bin/vga.o bin/ui.o bin/c8

main.o: src/main.cpp
	$(CC) $(CFLAGS) -o bin/main.o -c src/main.cpp

cpu.o: src/cpu.cpp
	$(CC) $(CFLAGS) -o bin/cpu.o -c src/cpu.cpp

memory.o: src/memory.cpp
	$(CC) $(CFLAGS) -o bin/memory.o -c src/memory.cpp

opcodes.o: src/opcodes.cpp
	$(CC) $(CFLAGS) -o bin/opcodes.o -c src/opcodes.cpp

vga.o: src/vga.cpp
	$(CC) $(CFLAGS) -o bin/vga.o -c src/vga.cpp

ui.o: src/ui.cpp
	$(CC) $(CFLAGS) -o bin/ui.o -c src/ui.cpp