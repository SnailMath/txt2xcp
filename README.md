# txt2xcp
_click on "CPhexEditor_V-1.0.0 Latest" on the right side of the screen to download the executable. -->_


This is a simple program to convert (text-) files to .xcp files for the calculator Classpad II (aka fx-cp 400).

Usage: `./txt2xcp [OPTIONS] SOURCE DEST`
Converts the file SOURCE to an .xcp file called DEST.
Options:
  -l       Convert newline Characters from "\r\n" (Windows) or "\n"
           (Linux) to "\r" (The newline characters used by the calc). Use
           this only for text, and don't use it for programs or binary data.
Examples:
./txt2xcp yourfile.bin newfile.xcp
./txt2xcp -l yourfile2.txt newfile2.xcp

The xcp file is of the type program(text). The variable will be called "file", when you import it on the calculator, but you can rename it (directly on the calc).
