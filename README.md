# ThunderOS

A hobby OS because all OS's suck, including mine.

### What does it do?

It... loads, kind of.

### What's Jai, and why are you using it?

It's a language. The Language, really. It's meant to be a low-level game programming language, written by Jonathan Blow, but I'm trying to use it for an OS. Wish me luck.

## How to build

1. Clone this repository
2. Install the Jai compiler and add it to your path
3. Install `gdb`, `qemu`, `qemu-nbd`, and `ld`
4. Make sure you have `mount`, `objcopy`, and `objdump` too
5. Run `sudo modprobe nbd`
6. Run `jai src/build.jai - build`

## How to run

6. Run `jai src/build.jai - run`

## Contributors

* Aria Seiler (Electro_593)

## License

[The Unlicense](LICENSE)