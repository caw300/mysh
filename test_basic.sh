#!/bin/sh
set -eu
make
printf 'pwd\nwhich ls\nwhich missing_command_123987\nexit\n' | ./mysh > /tmp/mysh_basic.out 2>/tmp/mysh_basic.err
printf 'Basic stdout:\n'; cat /tmp/mysh_basic.out
printf 'Basic stderr (expected missing executable):\n'; cat /tmp/mysh_basic.err
printf 'cd /\npwd\ncd /does-not-exist-12345\nexit\n' | ./mysh > /tmp/mysh_cd.out 2>/tmp/mysh_cd.err
printf 'cd stdout (expected /):\n'; cat /tmp/mysh_cd.out
printf 'cd stderr (expected error):\n'; cat /tmp/mysh_cd.err
printf 'pwd\nexit\n' > /tmp/mysh_input.txt
./mysh /tmp/mysh_input.txt
