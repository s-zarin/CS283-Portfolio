#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

# Single Pipe: Basic pipeline test
@test "Pipes: ls | grep '.c'" {
    run ./dsh <<EOF
ls | grep ".c"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dshlib.c"* ]]
}

# Single Pipe: Echo and grep test
@test "Pipes: echo 'hello world' | grep 'hello'" {
    run ./dsh <<EOF
echo "hello world" | grep "hello"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"hello world"* ]]
}

# Double Pipe: Three commands in a row
@test "Pipes: ls | grep '.c' | wc -l" {
    run ./dsh <<EOF
ls | grep ".c" | wc -l
EOF

    echo "Raw Output: '$output'"

    first_line=$(echo "$output" | head -n 1 | sed 's/^[[:space:]]*//') 
    echo "First Line (Processed): '$first_line'"

    [ "$status" -eq 0 ]
    [[ "$first_line" =~ ^[0-9]+$ ]]
}


# Pipe with nonexistent command
@test "Pipes: Invalid command in pipeline" {
    run ./dsh <<EOF
ls | nonexistentcmd | wc -l
EOF
    [[ "$status" -ne 0 || "$output" == *"execvp failed"* ]]
}

# Background process with a pipeline (check async behavior)
@test "Pipes: sleep 1 | echo 'done' (ensure shell waits for pipeline completion)" {
    run ./dsh <<EOF
sleep 1 | echo "done"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"done"* ]]
}

@test "Pipes: echo 'hello' | tr a-z A-Z | rev" {
    run ./dsh <<EOF
echo "hello" | tr a-z A-Z | rev
EOF
    first_line=$(echo "$output" | head -n 1 | tr -d '\n' | sed 's/[[:space:]]*$//')

    [ "$status" -eq 0 ]
    [[ "$first_line" == "OLLEH" ]]
}


# Redirects and pipes together (if redirection is implemented)
teardown() {
    rm -f output.txt
}
@test "Pipes & Redirection: echo 'test' | tee output.txt" {
    run ./dsh <<EOF
echo "test" | tee output.txt
EOF
    [ "$status" -eq 0 ]
    run cat output.txt
    [[ "$output" == "test" ]]
}

# Exit command should terminate shell gracefully
@test "Exit: Running exit should return 0" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"exiting..."* ]]
}