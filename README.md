# `gantry`

This gantry firmware was originally forked from [`Makeblock-official/XY-Plotter-2.0`](https://github.com/Makeblock-official/XY-Plotter-2.0), but has since been significantly re-written.

This repository contains the low-level machine control for our x-y stepper motors and limit switches for our pick-and-place machine.

## Usage

Firstly, clone this repository.

This repository is structured as a [PlatformIO](https://platformio.org/) project.

```sh
# git submodule update --init
code .
```

- Use `w`, `a`, `s`, `d`, `h`, `j`, `k`, or `l` to step the gantry a fixed increment in the specified direction.
- Use G-Code to provide a target location (`LF`-terminated!).

```sh
G28 # go home
G0 X10000 Y10000 Z0 # go to 10000um, 10000um, 0um
G1 X10000 Y10000 Z0 # ''
```

## Known Issue(s)

> [!warning]
> This list does not claim to be complete---it contains only the issues I can immediately recall.

1. If a target position is provided that exceeds the working range, the limit switches are hit, and the current position written to that invalid position. All future operations then have some offset error. Said differently, there is no bounds checking.
2. Feed rate `G1 F...` is not supported.
3. Commands may be ignored if provided too quickly; ie if the present command is yet to complete. Said differently, there is no handshaking.

<!-- ## Interfaces

### USB

- A USB serial port at `115200` baud is used to exchange data between this firmware and the [`p4p-83/controller`](https://github.com/p4p-83/controller).

#### Protocol Buffers

- [Protocol buffers](https://protobuf.dev/overview/) are used for data exchange.
- See [`p4p-83/protobufs`](https://github.com/p4p-83/protobufs) for the `.proto` definition(s). -->
