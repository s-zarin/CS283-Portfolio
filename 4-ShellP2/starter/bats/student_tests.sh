#!/usr/bin/env bats

# File: student_tests.sh
# 
# Comprehensive test suite for dsh

@test "Executes 'ls' and verifies output" {
    run ./dsh <<EOF
ls
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"dshlib.c"* ]]
}

@test "Exit command works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "cd to root and verify with pwd" {
    run ./dsh <<EOF
cd /
pwd
EOF
    first_line=$(echo "$output" | head -n 1)
    [[ "$first_line" == "/" ]]
}

@test "cd with no arguments does nothing" {
    run ./dsh <<EOF
pwd
cd
pwd
EOF
    output_lines=($(echo "$output" | tr '\n' ' '))
    [ "${output_lines[0]}" == "${output_lines[1]}" ]
}

@test "cd to a nonexistent directory fails" {
    run ./dsh <<EOF
cd /this_does_not_exist
EOF
    [[ "$output" == *"cd failed"* ]]
}

@test "cd to a relative path (..)" {
    run ./dsh <<EOF
pwd
cd ..
pwd
EOF
    output_lines=($(echo "$output" | tr '\n' ' '))
    [ "${output_lines[0]}" != "${output_lines[1]}" ]
}

@test "Empty input does nothing" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh2>"* ]]
}

@test "Handles quoted spaces correctly" {
    run ./dsh <<EOF
echo "hello,     world"
EOF
    [[ "$output" == *"hello,     world"* ]]
}



