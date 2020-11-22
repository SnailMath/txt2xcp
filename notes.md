I programmed this a few month ago, I didn't intended to publish that at that time. These are the notes I took about the file format casio uses.
The variable on the calc will be called 'file', because I didn't bother to adjust the checksum for different filenames.
I exported different files from the calculator (actually the cp manager) and compared the hexdump of them until I found this out:

```

0x4b length

0x61: length+3
0x62: length next digits (little endian)

0x6e: content of the file in blocks of 4 bytes

after the content:
00ff

0-3 bytes filled with 00 , fill until address kongruent to 0 mod 4

at next location kongruent to 1 mod 4:
2 bytes checksum
eof



i know how to generate this
|
| no of bytes
|   |          offset in the file
|   |     +---------+
|   |     |         |
*  0x48   0x00 - 0x47 : always the same
+  0x04   0x48 - 0x4b : length + 0x12 & ~0x03  big endian
*  0x0d   0x4c - 0x58 : "GUQ" + 0xff
+  0x08   0x59 - 0x60 : 8 digits ascii    length + 0x12 & ~0x03
*  0x0d   0x61 - 0x6d : length + 3 in little endian, followed by 0x00
*         0x6e -      : data
*  0x02               : 0x00 0xff
*  0-3                : ((0x01 - length) & 0x03) bytes of 0x00
   0x02               : checksum


```

I tried different ways for calculatng the checksum (from including certin  values and not including certain values, until I got it working. First this worked only for files less than 9 byte long, but I found the bug. This will work with files up to 32000 bytes and even more. (I tested it with chapters of random ebooks I had lying around.)
