#!/bin/bash

# execute echo trapped when SIGINIT signal happens
trap 'echo traped!' INT
# tell signal to die with code SIGINT
kill -s INT $$
# echo trapped executes after the function returns --maybe exec option is necessary for this case