# standalone-scp

https://github.com/bobg/scp
example scp in go.  Will be a good reference

# running
## 1. Build `stellar-core`
Clone `stellar-core` locally.
```
$ git clone https://github.com/stellar/stellar-core
```
Follow instructions on how to install `stellar-core` [https://github.com/stellar/stellar-core/blob/master/INSTALL.md](https://github.com/stellar/stellar-core/blob/master/INSTALL.md)

## 2. Edit makefile
In `standalone-scp/Makefile`, set `CORE_DIR?=` to the location of your built `stellar-core`

ex: in my `Makefile` I have `CORE_DIR?=$(HOME)/dev/cbdc/stellar-core`

## 3. Compile this repo
Run the following in `standalone-scp/`:

`$ make`

If you make changes to any files or want to re-compile:

`$ make clean` will remove any `.o` & `.exe` files

## 4. run
`$ ./main.exe`

# node-input.txt
This is the input file that defines the nodes and their trusted slices.
Input file needs to be in the root folder of this repo.
Format is as follows:

[node name]<br />
[names of trusted nodes to be added to node's slice.  Space separated]<br />
[empty line]<br />
[node name]<br />
[names of trusted nodes to be added to node's slice.  Space separated]<br />
[empty line]<br />
etc.
