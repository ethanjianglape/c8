# c8

c8 is an emulator for the [CHIP-8 microprosessor](https://en.wikipedia.org/wiki/CHIP-8).

You can find example CHIP-8 programs to run [here](https://github.com/kripod/chip8-roms/tree/master).

You can also use [this CHIP-8 compiler](https://github.com/glouw/c8c) to write and test programs

![Example Image](/assets/c8-example.png)

## Features

- [x] Pause and resume emulation (use -p flag to start the emulator in the paused state)
- [x] Execute CPU cycles one at a time
- [x] Go back one CPU cycle (up to 1,000 total)

## Todo

- [ ] Implement remaining CHIP-8 CPU [quirks](https://github.com/chip-8/chip-8-database/blob/master/database/quirks.json)
- [ ] Implement [SUPER-CHIP instructions](https://chip-8.github.io/extensions/)
- [ ] Add cross platform config file for things like pixel size/color, CPU frequency, etc.
