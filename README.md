mprpc-bench
===========

Benchmarks for kohler/mprpc for Harvard CS260r

## Building

You first need to pull the mprpc and tamer submodules and build them.

    git submodule update --init --recursive

The `mprpc` folder includes a README for itself.

There are four make targets, one for compiling an executable for each test.

    paxos
    message-size
    windowed-message-size
    multi-client

The invocation `make FOO` should successfully compile any of them.

There are four corresponding scripts which we used to generate our data.

Each source fiel has a Clp_Option struct which lists the possible command line
arguments to the executable.
