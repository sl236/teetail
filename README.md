# teetail

## what?

teetail is like tee - it echoes its standard input to standard output and also to a file; but also like tail - no more than the number of bytes specified on the command line are placed in the file, and these are the tail of the dataset passing through teetail.

## why?

You have a job producing gigabytes of output. You want to watch it scroll by on your terminal. You also want to poke through the tail end of it when it finishes; you want enough context to work out what was happening when it finished, but more than the terminal retains, yet you don't have enough disk space for the whole thing.

## how?

*some pipeline* `| teetail -o log -c 1048576 |` *more pipeline*

There are some configuration options, `teetail` with no arguments to see them.

## compile

use your favourite C compiler, e.g.

`gcc teetail.c -o teetail`


## install

place the `teetail` binary wherever locally built things go on your system, e.g.

`sudo mv teetail /usr/local/bin`

