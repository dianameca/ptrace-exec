# Helper for bat tests

check() {
	res=0
	if [ "$status" != "$1" ]; then
		echo "Failed tests: the return code is $status ; $1 was expected"
		echo "The exit is :"
		echo "$output"
		res=1
	fi

	if [ "$output" != "$2" ]; then
		echo "Failed test: the unexpected exit is :"
		echo "$output"
		echo ""
		echo "The diff is"
		diff -u <(echo "$2") <(echo "$output")
		res=2
	fi

	return "$res"
}

checki() {
	out=`cat`
	check "$1" "$out"
}

setup() {
	true
}

teardown() {
	kill %% 2> /dev/null
	pchild=`pgrep -s 0 sleep`
	kill $pchild 2> /dev/null
	true
}

# Run test with active `BATS_VALGRIND` to detect memory leaks
if [[ -n "$BATS_VALGRIND" ]]; then
	eval orig_"$(declare -f run)"
	run() {
		orig_run valgrind -q --leak-check=full "$@"
	}
fi
