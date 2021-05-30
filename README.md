Pariwat Huang 6180067 IC-SHELL

References:
https://github.com/szholdiyarov/command-line-interpreter/blob/master/myshell.c
https://stackoverflow.com/questions/58361506/save-history-command-on-simple-shell-by-c-code
https://stackoverflow.com/questions/52939356/redirecting-i-o-in-a-custom-shell-program-written-in-c
http://people.cs.pitt.edu/~khalifa/cs449/spr07/Assigns/Assign4/myshell.c
https://github.com/hungys/mysh/blob/master/mysh.c
https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html


Features:

```echo``` command for printing text to screen.

```shell
icsh> echo Hello To the Shell
Hello To the Shell
```

`exit` command for exit the shell.

```shell
icsh> exit 1
Bye
```

`!!` command to repeat previous command.

```shell
icsh> echo hello world
hello world
icsh> !!
hello world
```

Script Mode:

Shell can read file that come from the argument for input.

```shell
## test.sh

echo hello
echo world
!!
exit 5
```



```shell
$ ./icsh test.sh
hello
world
world
Bye
```

External Program:
``` icsh $ ls
file_a file_b file_c
icsh $
```

I/O Redirection:
```
icsh $ la -l > some_file
```

Background jobs and jobs control:
```
1: icsh $ sleep 5 &
2: [1] 843
3: icsh $
```

Jobs:
```
icsh $ sleep 100 &
[1] 855
icsh $ sleep 200 &
[2] 856
icsh $ jobs
[1]-  Running                 sleep 100 &
[2]+  Running                 sleep 200 &
icsh $ 
```

Foreground fg %<job_id>:
```

icsh $ sleep 100 &
[1] 862
icsh $ fg %1
sleep 100
```

Background bg %<job_id>:
```

icsh $ sleep 100
^Z
[1]+  Stopped                 sleep 100
icsh $ bg %1
[1]+ sleep 100 &
icsh $ 
```

 
##Additional Feature:

#### reminder
- type `remindme <no of seconds> "What you are going to do"` to set a reminder
- This feature will have a glitch after exiting the shell 
- Intended not to add to the main background process in order to avoid internal problem
