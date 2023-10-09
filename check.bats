#!/usr/bin/env bats

load test_helper

@test "hello" {
	run ./extra echo hello world
	checki 0 <<FIN
/usr/bin/echo
hello world
FIN
}

