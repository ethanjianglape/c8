# c8

c8 is an emulator for the [CHIP-8 microprosessor](https://en.wikipedia.org/wiki/CHIP-8).

You can find example CHIP-8 programs to run [here](https://github.com/kripod/chip8-roms/tree/master).

You can also use [this CHIP-8 compiler](https://github.com/glouw/c8c) to write and compile programs.

![Example Image](/assets/c8-example.png)

## Dependencies

1. [sfml](https://www.sfml-dev.org/)
2. c++20

## Building
### Linux

Ensure that `libsfml-dev` is installed on your machine and run the following command:
```
make
```
### Windows
Open `c8vs.sln` in visual studio.

Ensure [sfml](https://www.sfml-dev.org/tutorials/2.6/start-vc.php) is installed and configured properly for visual studio.

By default, c8vs expects sfml to be installed in `C:\SFML-2.6.1\`.
## Running

Running `./bin/c8` by itself will start the emulator with a default program loaded into memory that simply prints "C8" onto the screen.

To run a different CHIP-8 executable, run with `./bin/c8 yourProgramName.bin`.

To start the emulator in a paused state, use the parameter `-p`:

```
./bin/c8 -p yourProgramName.bin
```

## Features

- [x] Pause and resume emulation (use -p flag to start the emulator in the paused state)
- [x] Execute CPU cycles one at a time
- [x] Go back one CPU cycle (up to 1,000 total)
- [x] Linux and Windows support

## Todo

- [ ] Implement remaining CHIP-8 CPU [quirks](https://github.com/chip-8/chip-8-database/blob/master/database/quirks.json)
- [ ] Implement [SUPER-CHIP instructions](https://chip-8.github.io/extensions/)
- [ ] Add additinal control flags and a `-help` message
