# nflate

nflate is a naive implementation in C of the decompression (inflation) side of the [DEFLATE](https://en.wikipedia.org/wiki/DEFLATE) algorithm that can unzip [gzip](https://en.wikipedia.org/wiki/Gzip) files. It is mainly based on RFC 1951 (DEFLATE) and RFC 1952 (gzip). It is not trying to be performant. I just wrote it as a means to learn more about compression algorithms. It's probably of little use to others since there are better examples to learn from. I have listed some learning resources that I used below.

Wherever possible I used the original variable names from RFC 1951/RFC 1952 in their original capitalization. That is why there is some strange naming of variables throughout the code.

Please don't use this code in a production system. There is no reason to do that. There are many great freely licensed, more performant, and better tested implementations of DEFLATE including [zlib](https://zlib.net).

## Using

You can compile it on a system with GNUMake and a gcc compatible C compiler (e.g. macOS or GNU/Linux) using
```
make debug
```
or
```
make release
```

You can compile it on Windows with Microsoft's build tools including nmake installed and
```
nmake debug
```
or
```
nmake release
```

There is also an included Xcode project that uses GNUMake as an external build system.

You can then run it and provide the name of the gzip file you want to decompress.

```
./nflate samples/house.jpg.gz
```
or on Windows

```
nflate samples\house.jpg.gz
```

You can optionally specify the name of the output file after the name of the compressed file.

## Testing

There's a bash script `test_correctness.sh` that will try decompressing the gzipped files in the `samples` folder and compare them to their originals using `diff`. It is what is automatically run by a GitHub Action here. Unfortunately, I couldn't find (or easily generate) any gzip files compressed with the fixed type block type. So, that block type is untested...

## External Resources

I used several external resources to learn more about DEFLATE/gzip to create this sample project.

- [RFC 1951](https://tools.ietf.org/html/rfc1951) official DEFLATE format spec, my main guide
- [RFC 1952](https://tools.ietf.org/html/rfc1952) official gzip format spec
- [Dissecting the GZIP format](http://commandlinefanatic.com/cgi-bin/showarticle.cgi?article=art001) great article by Joshua Davies that helped me understand some of the details
- [puff](https://github.com/madler/zlib/tree/master/contrib/puff) another naive implementation of DEFLATE decompression by one of the authors of zlib/gzip, Mark Adler; I modified this code to print out its intermediary tables to find a couple bugs in my code
- [DEFLATE](https://en.wikipedia.org/wiki/DEFLATE) on Wikipedia
- [gzip](https://en.wikipedia.org/wiki/Gzip) on Wikipedia
- [Cyclic Redundancy Check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check) on Wikipedia

## License

I wrote almost all of the code in this naive implementation. I am releasing it under the Apache License 2.0 (see `LICENSE`) which means you can use it as you would like to as long as you post the license with it and give me credit. Some very small snippets are based on pseudocode from Wikipedia or code from RFC 1951. They are marked as such where they occur. The very small portion of code from RFC 1951 is under the following license:

```
Copyright (c) 1996 L. Peter Deutsch

Permission is granted to copy and distribute this document for any
purpose and without charge, including translations into other
languages and incorporation into compilations, provided that the
copyright notice and this notice are preserved, and that any
substantive changes or deletions from the original are clearly
marked.
```
