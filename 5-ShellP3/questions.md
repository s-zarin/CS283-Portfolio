1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

I use waitpid() inside a loop. After forking each child process to execute a command in the pipeline, the parent process waits for all child processes to terminate before displaying the next shell prompt. If I forgot to call waitpid(), then zombie processes would occur, and the shell would be accepting and trying to execute commands before they're fully finished.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Its necessary since not closing unused pipe ends can cause resource leaks and unpredictable pipe behavior. Open pipes can lead to processes not fully finishing and waiting indefinitely, along with unecessary consumption of file descriptors.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

cd is implemented as a built-in because changing directories affects the working directory of the shell process. If it was implemented as an externl command, then it would run in a child shell and any changes would happen there, not affecting the parent shell at all.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

I would modify my program to dynamically allocate an array of pipes instead of using a fixed size one. It would allow a lot more flexibility and no limits on the number of piped commands. It would be very complex though and consume a lot more memory usage than a fixed size pipe array.
