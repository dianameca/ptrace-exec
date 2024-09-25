# ptrace-exec

This is a small tool that uses `ptrace` to monitor the execution of a program and its uses of the `execve` system call.

## Description

`extra command [arguments...]`

`extra`  - *exec trace* - displays the full paths of the programs executed by a command and its subprocesses.

Every time an `exec` occurs, the complete path of the new executable is printed to the standard error output.

Example of `extra` listing the programs used for generating a manpage in postscript format :

```
$ extra man -Tps man > man.ps
/usr/bin/man
/usr/bin/preconv
/usr/bin/tbl
/usr/bin/groff
/usr/bin/troff
/usr/bin/grops
```

## Features

The first path displayed is the path of the command's program.

```
$ extra echo hello world
/bin/echo
hello world
```

The path of the program that is executed is actually being showed.   
In the case of a script it displays the interpreter's path :

```
$ extra ./script.sh
/bin/bash
hello world
```

Below the first `echo` is an internal shell command (no `exec` occurs) while the second `echo`is the standalone program `/bin/echo`.

```
$ extra env M=world nice -n 10 bash -c 'echo hello $M; exec echo goodbye $M'
/usr/bin/env
/usr/bin/nice
/bin/bash
hello world
/bin/echo
goodbye world
```

Subprocesses are also automatically tracked.

```
$ extra time -p echo hello world
/usr/bin/time
/bin/echo
hello world
real 0.00
user 0.00
sys 0.00
```

## Return Code

The return code is the same as the command executed. 

If command cannot be executed -> returns 127. 

If command is terminated by a signal -> returns 128 plus the signal number.


## Running tests

On linux make sure `bats`, `make` and `gcc` are installed (`sudo apt-get install` ...)

`make check` to run the tests.
