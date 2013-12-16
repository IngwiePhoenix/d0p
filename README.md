# drag0n Package

Archives with descriptions.

## d0p - what's that?

A drag0n package is like a box with a label. A header is wrapped around an actual archive file. We use libarchive and lzma in order to compress TAR+XZ archives, and drop compressed YAML around, and use it as a label. 

## How it works

The actual structure is really easy:

    Block 1              Block 2
    
    1 2 3 4 5 6 7 8 | 9 10 11 12 13 14 15 16    Byte
    R R R R R R R R   R  R  R  R  R  0  0  0    Random Key
    ... INFO ...                                header information
    0 0 0 R R R R R   R  R  R  R  R  R  R  R    Random Key
    ... DATA ...                                actual files, archived

In here, R means a RANDOM byte, mostly ASCII characters - I used `uniqid()` from PHP when I made this first.

The procedure to extract things is as follows:

1. Read the first 13 bytes and verify that the following three bytes are NULL (\0).
2. Store the key, and start seeking byte after byte untill you discover three NULL bytes followed by the 13 byte key.
3. Anything between the first and second occurence of the key is the header. It will be compressed.
	- Anything AFTER the second key is the archive itself.
4. Read everything after the second key occurence and stream the data to disk. It will be a .tar.xz archive.
5. Decompress the header. You now have "unboxed" (or as the code says "unwrap") the package, and now have its headers and the actual archive right there.


## For what is it?

d0p's can be used for all sorts of things - like if you create a backup, you can add comments to the headers. Or if you distribute your software, you can add a diversity of information to the head.

## Status?

I got all the needed libraries and stuff together now, and a nice build script. Now I just need to code d0p itself! :3

## for what will YOU use it?

I will use it within my communtiy software in order to create backups of things and inside my package manager drag0n. It will take advantage of the header and store package information right in there. By then, we can even use basic PHP and iterate over it:

```PHP
<?php // pseudo code
foreach(scandir("pkgdir") as $pkg) {
	$p = fopen($pkg, "r");
	$key = fread($p, 13);
	$seq = fread($p, 3);
	if($seq != "\0\0\0") trigger_error("Package <$pkg> is invaild.");
	$out=null;
	while($byte = fread($p, 1)) {
		$curByte = ftell($p);
		if($byte == "\0" && fread($p,15) == "\0\0$key") {
			// Found key and sequence.
			break;
		} else {
			// Reset to the actual position, so it can be moved once forward int he next sequence.
			fseek($p,$curByte);
			$out.=$byte;
		}
	}
	d0()->DoSomethingWithHeader($out);
}
```

# Have fun :D
