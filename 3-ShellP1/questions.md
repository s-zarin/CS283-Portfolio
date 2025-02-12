1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is good because as a command it does what we need it to do: read a single line and stop at a /n or EOF while appending a null terminator. This is perfectly suited for line by line input in a shell. fgets also doesn't cause buffer issues like scanf can.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  malloc() gives more dynamic control over fixed size arrays, and allows us to scale the buff to the input rather than capping it at an arbitrary amount. This would be fine for a starter shell but to make the shell easy to expand malloc() is best and more flexible.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Not trimming whitespace can lead to a lot of issues with command interpretation. Its possible the shell might interpret the whitespace as literal commands and return "command not found" errors or see it as empty tokens

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  1. command > example.txt - This is output redirection to a specific file. There can be challenges with making sure youre creating the correct file or overwriting existing data ;2. command < file.txt - this redirects input from a file. A challenge of what to do when the file itself doesn't exist and what type of error to throw (or not throw) ; 3. command >> example.txt - This appends the output to the end of the file without overwriting. There can be a possible challenge of excess file size.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection is writing and/or reading from a file, while piping feeds a commands output into another commands input without a file being involved.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  We don't want to store and output our errors with normal output so that the user doesn't have issues separating problems from standard output, which is helpful when debugging or figuring out what went wrong with a command.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  By default both STDERR and STDOUT should remain separate so the user has a choice to see normal output in one place and errors in the other. We could put in an option so that users can choose to redirect or merge them but the standard should be to keep things separate.