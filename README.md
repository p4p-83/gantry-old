# `gantry`

This gantry firmware was originally forked from [`Makeblock-official/XY-Plotter-2.0`](https://github.com/Makeblock-official/XY-Plotter-2.0), but has since been significantly re-written.

It is presently in a half-finished, never-to-be-finished state---the decision is to re-write from scratch.

This documentation is written for posterity.

## Usage

- Use `w`, `a`, `s`, `d`, `h`, `j`, `k`, or `l` to step the gantry a fixed increment in the specified direction.
- Use G-Code to provide a target location (`LF`-terminated!).

```sh
G28 # go home
G0 X10000 Y10000 Z0 # go to 10000um, 10000um, 0um
G1 X10000 Y10000 Z0 # ''
```

## Known Issue(s)

> [!warning]
> This list does not claimed to be complete---it contains only the issues I can immediately recall

1. If a target position is provided that exceeds the working range, the limit switches are hit, and the current position written to that invalid position. All future operations then have some offset error. Said differently, there is no bounds checking.
2. Feed rate `G1 F...` is not supported.
3. Commands may be ignored if provided too quickly; ie if the present command is yet to complete. Said differently, there is no handshaking.
