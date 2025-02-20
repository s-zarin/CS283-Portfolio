1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Using fork() before execvp() lets the parent process (the shell) remain active and responsive while the child process handles the execution of external commands, if we called execvp() directly without forking, the shell  would be replaced by the new program, and we would lose the whole shell process

The value of fork() is that it creates a separate child process, so the shell can continue accepting commands even while a command is running. This is what makes multitasking in a shell possible.

1. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() fails, it returns -1, meaning the system couldn’t create a new process. In my implementation I make it so the shell prints an error message and returns to the prompt to avoid crashes

2. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() searches for the command in the directories listed in $PATH, if its absolute command then it executes it on its own but if its relative it searches through directories in $PATH to find a match.

3. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  Calling wait() ensures the parent (shell) waits for the child process (command) to finish before accepting any new input. If we don’t call wait(), the shell would immediately return to the prompt without waiting for the command to finish leaving executed processes that haven't been cleaned up in the shell

4. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS(status) tells us the exit status of the child process after it finishes executing, and its helpful because some commands fail and don't provide any indication they did. If we check all the exit codes we can see which commands have failed even if they fail silently

5. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My implementation preserves spaces inside quoted strings while collapsing extra spaces outside quotes

6. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  In Assignment 3, I split the input based on pipes (|) and used build_cmd_list() to generate a list of commands in a command_list_t structure. Each command was stored with a separate exe and args field. Whenever I encountered a pipe, I’d treat it as a brand-new command, which I then appended to the list. That was all well and good, except it didn’t really handle quotes or spaces in a nuanced way. In Assignment 4 I refactored the parsing logic in a simpler, more direct approach for a single command (no piping right now). Instead of build_cmd_list(), I use a function called parse_input() that handles quoted strings. Anything wrapped in double quotes stays as a single argument—even if it has spaces. Outside quotes, consecutive spaces compress into a single delimiter. Also, I got rid of all that pipe-based logic since this week’s assignment doesn’t need multiple commands or piping. Now, I just populate cmd_buff_t cmd with argv[] and keep track of argc. In terms of challenges, I had to preserve all the spaces within quotes so I needed to toggle a bool and rewrite a lot of the logic in my code. I had to change memory management as well since I'm using malloc and strdup, then freeing everything. It was mostly tough to remove the pipe logic and keep everything else intact that depended on the previous code.

7. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are asynchronous notifications that the OS sends to a process to communicate an event (like termination requests, segmentation faults, or user interrupts). Unlike other IPCs, signals are unidirectional and don’t carry complex data

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL: Immediately terminates a process without cleanup; SIGTERM: Requests a process to terminate; SIGINT: Interrupts a running process.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: SIGSTOP pauses the process indefinitely until a SIGCONT signal is sent
