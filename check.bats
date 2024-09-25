#!/usr/bin/env bats

load test_helper

@test "hello" {
	run ./extra echo hello world
	checki 0 <<FIN
/usr/bin/echo
hello world
FIN
}

@test "script ✓" {
	run ./extra ./script.sh
	checki 0 <<FIN
/usr/bin/bash
hello world
FIN
}

# makes 5 execve calls 
# program stops after 2...
@test "nice ✓" {
	run ./extra nice echo hello world
	checki 0 <<FIN
/usr/bin/nice
/usr/bin/echo
hello world
FIN
}

@test "env, nice, etc. ✓" {
	run ./extra env M=world nice -n 10 bash -c 'echo hello $M; exec echo bye $M'
	checki 0 <<FIN
/usr/bin/env
/usr/bin/nice
/usr/bin/bash
hello world
/usr/bin/echo
bye world
FIN
}

# clone event
@test "time" {
	run ./extra time -p echo hello world
	checki 0 <<FIN
/usr/bin/time
/usr/bin/echo
hello world
real 0.00
user 0.00
sys 0.00
FIN
}

# clone event
@test "env, time, etc." {
	run ./extra env B=olleh time -p dash -c 'nice echo $B | rev'
	output=$(echo "$output" | LC_ALL=C sort)
        checki 0 <<FIN
/usr/bin/dash
/usr/bin/echo
/usr/bin/env
/usr/bin/nice
/usr/bin/rev
/usr/bin/time
hello
real 0.00
sys 0.00
user 0.00
FIN
}

# clone event
#@test "man" {
#	run bash -c './extra man -Tps man > man.ps'
#	output=$(echo "$output" | LC_ALL=C sort)
#        checki 0 <<FIN
#/usr/bin/groff
#/usr/bin/grops
#/usr/bin/man
#/usr/bin/preconv
#/usr/bin/tbl
#/usr/bin/troff
#FIN
#}

@test "trap ✓" {
	run ./extra ./trap.sh
	checki 0 <<FIN
/usr/bin/bash
traped!
FIN
}

@test "exit ✓" {
	run ./extra bash -c 'exit 10'
	checki 10 <<FIN
/usr/bin/bash
FIN
}

@test "kill ✓" {
	run ./extra bash -c 'kill $$'
	checki 143 <<FIN
/usr/bin/bash
FIN
}

@test "fail ✓" {
	run ./extra failfailfail
	test "$status" == 127
	test "$output" != ""
}
