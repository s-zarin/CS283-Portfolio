#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int count_words(char *, int);
int reverse_string(char *, int);
int word_print(char *, int);
int find_replace(char *, int, char *, char *, int);


//TODO: #4:  Implement the setup buff as per the directions
int setup_buff(char *buff, char *user_str, int len) {
    int count = 0;
    int found_space = 0;
    char *dest_ptr = buff;
    char *src_ptr = user_str;

    // Preliminary length check to ensure input fits in buffer
    int src_length = 0;
    char *temp_ptr = user_str;
    while (*temp_ptr != '\0') {
        src_length++;
        temp_ptr++;
    }
    if (src_length > len) {
        return -1; // Input string too large
    }

    while (*src_ptr != '\0') {
        if (count >= len) {
            return -1; // Buffer overflow
        }
        if (*src_ptr == ' ' || *src_ptr == '\t' || *src_ptr == '\n') {
            if (!found_space && count > 0) {
                *dest_ptr = ' ';
                dest_ptr++;
                count++;
                found_space = 1;
            }
        } else {
            *dest_ptr = *src_ptr;
            dest_ptr++;
            count++;
            found_space = 0;
        }
        src_ptr++;
    }

    // Remove trailing space if present
    if (found_space && count > 0) {
        dest_ptr--;
        count--;
    }

    // Pad the buffer 
    for (int i = count; i < len; i++) {
        *(buff + i) = '.';
    }

    return count;
}

void print_buff(char *buff, int len) {
    printf("Buffer:  [");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    printf("]\n");
}

void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int str_len) {
    int word_count = 0;
    int in_word = 0;
    for (int i = 0; i < str_len; i++) {
        char c = *(buff + i);
        if (c == ' ') {
            in_word = 0;
        } else {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        }
    }
    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

// reverse the string in the buffer
int reverse_string(char *buff, int str_len) {
    if (str_len < 1) return -1;
    int start = 0;
    int end = str_len - 1;

    while (start < end) {
        char temp = *(buff + start);
        *(buff + start) = *(buff + end);
        *(buff + end) = temp;
        start++;
        end--;
    }
    return 0;
}

// print each word in the buffer with its length
int word_print(char *buff, int str_len) {
    if (str_len < 1) return -1; // handle empty input

    int word_index = 1;
    int in_word = 0;
    int length_of_word = 0;
    char *word_start = NULL;

    printf("Word Print\n");
    printf("----------\n");

    for (int i = 0; i < str_len; i++) {
        char c = *(buff + i);
        if (c == ' ') {
            if (in_word) {
                // print current word if space present
                printf("%d. ", word_index++);
                for (int j = 0; j < length_of_word; j++) {
                    putchar(*(word_start + j));
                }
                printf("(%d)\n", length_of_word);

                in_word = 0;
                length_of_word = 0;
            }
        } else {
            if (!in_word) {
                // mark the start of new word
                in_word = 1;
                word_start = (buff + i);
                length_of_word = 1;
            } else {
                length_of_word++;
            }
        }
    }

    // print the last word if string doesnt end with space

    if (in_word && length_of_word > 0) {
        printf("%d. ", word_index++);
        for (int j = 0; j < length_of_word; j++) {
            putchar(*(word_start + j));
        }
        printf("(%d)\n", length_of_word);
    }

    return (word_index - 1);
}

// find and replace a substring in the buffer
int find_replace(char *buff, int str_len, char *old_str, char *new_str, int buff_size) {
    int old_len = 0;
    int new_len = 0;

    // calculate lengths of substring
    while (*(old_str + old_len) != '\0') {
        old_len++;
    }
    while (*(new_str + new_len) != '\0') {
        new_len++;
    }
    if (old_len == 0) {
        return -1; // invalid input
    }

    for (int i = 0; i <= str_len - old_len; i++) {
        int match = 1;
        // check for match
        for (int j = 0; j < old_len; j++) {
            if (*(buff + i + j) != *(old_str + j)) {
                match = 0;
                break;
            }
        }
        if (match) {
            int shift_amount = new_len - old_len;
            // check for overflow
            if (str_len + shift_amount > buff_size) {
                // pad buff
                for (int k = 0; k < buff_size - str_len; k++) {
                    *(buff + i + k) = *(new_str + k);
                }
                for (int k = buff_size - str_len; k < buff_size; k++) {
                    *(buff + k) = '.';
                }
                return -2;
            }
            // shift characters, insert new substring
            for (int k = str_len; k >= i + old_len; k--) {
                *(buff + k + shift_amount) = *(buff + k);
            }
            for (int k = 0; k < new_len; k++) {
                *(buff + i + k) = *(new_str + k);
            }
            str_len += shift_amount;
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    char *buff;         //placehoder for the internal buffer
    char *input_string; //holds the string provided by the user on cmd line
    char opt;           //used to capture user option from cmd line
    int rc;             //used for return codes
    int user_str_len;   //length of user supplied string



    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      if argv[1] didnt exist, the program would try to access an invalid memory location,
    //      leading to a segmentation fault. This check prevents possible segmentation faults

    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1); //get the option flag


    //handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);

    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS


    //TODO:  #2 Document the purpose of the if statement below
    //      Checks if required input string is missing, and if it is
    //      displays the usage message and exits
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = (char *)malloc(BUFFER_SZ * sizeof(char));
    if (!buff) {
        printf("Memory allocation failed.\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);   //see todos
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(3);
    }

    //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
    //       the case statement options
    switch (opt) {
        case 'c': {
            rc = count_words(buff, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            printf("Word Count: %d\n", rc);
            print_buff(buff, BUFFER_SZ);
            break;
        }
        case 'r': {
            rc = reverse_string(buff, user_str_len);
            if (rc < 0) {
                printf("Error reversing string, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            // Print only the reversed user string without the dot padding
            printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++) {
                putchar(*(buff + i));
            }
            printf("\n");
            print_buff(buff, BUFFER_SZ);
            break;
        }
        case 'w': {
            rc = word_print(buff, user_str_len);
            if (rc < 0) {
                printf("Error printing words, rc = %d\n", rc);
                free(buff);
                exit(3);
            }
            print_buff(buff, BUFFER_SZ);
            break;
        }
        case 'x': {
            if (argc < 5) {
                printf("Error: -x requires string, old, new arguments.\n");
                free(buff);
                exit(1);
            }
            rc = find_replace(buff, user_str_len, argv[3], argv[4], BUFFER_SZ);
            if (rc == 0) {
                printf("Modified String: ");
                for (int i = 0; i < BUFFER_SZ; i++) {
                    if (*(buff + i) == '.') break;
                    putchar(*(buff + i));
                }
                putchar('\n');
            } else if (rc == 1) {
                printf("Error: '%s' not found in the input string.\n", argv[3]);
            } else if (rc == -2) {
                printf("Buffer overflow or insufficient space for replacement.\n");
            } else {
                printf("Not Implemented or error with replacement.\n");
            }
            print_buff(buff, BUFFER_SZ);
            break;
        }
        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          Providing the pointer and the length ensures the functions are reusable aren't dependent on hardcoded sizes
//          It also prevents accidental buffer overflows in case the buffer size changes in the future 

