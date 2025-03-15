#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

SERVER_IP="127.0.0.1"
PORT="7890"

setup() {
    ./dsh -s -i $SERVER_IP -p $PORT &
    SERVER_PID=$!
    sleep 1
}

teardown() {
    echo "stop-server" | ./dsh -c -i $SERVER_IP -p $PORT
    wait $SERVER_PID 2>/dev/null
}

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Server starts and accepts client connections" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "echo test_connection"
    [ "$status" -eq 0 ]
    [[ "$output" == *"test_connection"* ]]
}

@test "Client can execute remote commands" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "ls"
    [ "$status" -eq 0 ]
    [[ "$output" == *"dshlib.c"* ]]
}

@test "Client can execute piped commands remotely" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<EOF
ls | grep dshlib.c
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dshlib.c"* ]]
}

@test "Client can change directories remotely" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "cd /tmp && pwd"
    [ "$status" -eq 0 ]
    [[ "$output" == "/tmp" ]]
}

@test "Server handles multiple clients sequentially" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "echo client1"
    [[ "$output" == *"client1"* ]]

    run ./dsh -c -i $SERVER_IP -p $PORT <<< "echo client2"
    [[ "$output" == *"client2"* ]]
}

@test "Server handles multiple commands in a session" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<EOF
echo first_command
echo second_command
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"first_command"* ]]
    [[ "$output" == *"second_command"* ]]
}

@test "Server sends EOF character correctly" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "echo test"
    [[ "$output" == *"test"* ]]
    [[ "$output" == *$'\x04' ]]  # Checking for EOF marker
}

@test "Server recognizes and executes built-in commands" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "cd / && pwd"
    [[ "$output" == "/" ]]
}

@test "Server handles exit command correctly" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "exit"
    [ "$status" -eq 0 ]
}

@test "Server stops gracefully on stop-server command" {
    run ./dsh -c -i $SERVER_IP -p $PORT <<< "stop-server"
    [ "$status" -eq 0 ]
}