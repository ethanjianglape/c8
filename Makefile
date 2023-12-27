CC = g++
CFLAGS = -std=c++20 -O3 -Wall -Wextra
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system

c8: main.o cpu.o memory.o opcodes.o vga.o ui.o
	$(CC) -o bin/c8 bin/*.o $(LDLIBS)

clean:
	rm bin/main.o bin/cpu.o bin/memory.o bin/opcodes.o bin/vga.o bin/ui.o bin/c8

main.o: main.cpp
	$(CC) $(CFLAGS) -o bin/main.o -c main.cpp

cpu.o: cpu.cpp
	$(CC) $(CFLAGS) -o bin/cpu.o -c cpu.cpp

memory.o: memory.cpp
	$(CC) $(CFLAGS) -o bin/memory.o -c memory.cpp

opcodes.o: opcodes.cpp
	$(CC) $(CFLAGS) -o bin/opcodes.o -c opcodes.cpp

vga.o: vga.cpp
	$(CC) $(CFLAGS) -o bin/vga.o -c vga.cpp

ui.o: ui.cpp
	$(CC) $(CFLAGS) -o bin/ui.o -c ui.cpp