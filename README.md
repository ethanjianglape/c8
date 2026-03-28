# c8

c8 is an emulator for the [CHIP-8 microprocessor](https://en.wikipedia.org/wiki/CHIP-8).

You can find example CHIP-8 programs to run [here](https://github.com/kripod/chip8-roms/tree/master).

You can also use [this CHIP-8 compiler](https://github.com/glouw/c8c) to write and compile programs.

![Example Image](/assets/c8-example.png)

## Dependencies

- [SFML 3](https://www.sfml-dev.org/)
- C++20

## Building

Ensure `libsfml-dev` is installed, then:

```
cmake -B build
cmake --build build
```

The executable will be at `build/bin/c8`.

## Running

Running `./build/bin/c8` by itself will start the emulator with a default program loaded into memory that prints "C8" onto the screen.

To run a CHIP-8 program:

```
./build/bin/c8 yourProgram.bin
```

To start paused, use the `-p` flag:

```
./build/bin/c8 -p yourProgram.bin
```

## Features

- Pause and resume emulation at any time
- Step through CPU cycles one at a time
- Rewind execution up to 1,000 cycles
- Real-time CPU frequency and FPS display
- Start paused with the `-p` flag
