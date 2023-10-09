all: extra

check: all
	bats check.bats

local: all
	bats local.bats
