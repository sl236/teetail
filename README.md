# teetail

[![.github/workflows/main.yml](https://github.com/sl236/teetail/actions/workflows/main.yml/badge.svg)](https://github.com/sl236/teetail/actions/workflows/main.yml)

## what?

`teetail` is like `tee` - it echoes its standard input to standard output and also to a file; but also like `tail` - no more than the number of bytes specified on the command line are placed in the file, and these are the tail of the dataset passing through `teetail`.

## why?

You have a pipeline moving gigabytes of output. You want to copy the data at some point in the pipeline to a file for debugging, like `tee` does. But there's way too much of it to store, so you want to limit the file size and just have the tool put the last part  in there, like `tail` would. You could probably construct some kind of Rube Goldberg thing out of `tee` and `tail` and `mkfifo`. Or you could use `teetail`.

## how?

*some pipeline* `| teetail -o log -c 1048576 |` *more pipeline*

There are some configuration options, `teetail` with no arguments to see them.

## compile

use your favourite C compiler, e.g.

`gcc teetail.c -o teetail`


## install

place the `teetail` binary wherever locally built things go on your system, e.g.

`sudo mv teetail /usr/local/bin`

