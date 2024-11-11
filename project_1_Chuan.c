/* Program to generate hierarchical reports from TSV structured data

   Skeleton program written by Alistair Moffat, ammoffat@unimelb.edu.au,
   August 2024, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.
   All included code is (c) Copyright University of Melbourne, 2024

   Student Authorship Declaration:

   (1) I certify that except for the code provided in the initial skeleton
   file, the  program contained in this submission is completely my own
   individual work, except where explicitly noted by further comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students, or by
   non-students as a result of request, solicitation, or payment, may not be
   submitted for assessment in this subject.  I understand that submitting for
   assessment work developed by or in collaboration with other students or
   non-students constitutes Academic Misconduct, and may be penalized by mark
   deductions, or by other penalties determined via the University of
   Melbourne Academic Honesty Policy, as described at
   https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will I
   do so until after the marks are released. I understand that providing my
   work to other students, regardless of my intention or any undertakings made
   to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring service,
   or drawing the attention of others to such services and code that may have
   been made available via such a service, may be regarded as Student General
   Misconduct (interfering with the teaching activities of the University
   and/or inciting others to commit Academic Misconduct).  I understand that
   an allegation of Student General Misconduct may arise regardless of whether
   or not I personally make use of such solutions or sought benefit from such
   actions.

   Signed by: [Yuchuan Zou 1431128]
   Dated:     [01/09/2024]

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>

#define CHAR_CR '\r'	     /* CR character, in PC-format text files */
#define CHAR_NL '\n'         /* newline character, ends each input line */
#define CHAR_TB '\t'         /* tab character, ends each TSV field */

#define STATUS_EOL 1         /* return value for end of line field */
#define STATUS_EOF 2         /* return value for end of file field */
#define STATUS_NML 3         /* return value if neither of previous two */

#define MAXFIELDLEN 50       /* maximum field length */
#define LINE_MAX 1000        /* maximum of input lines */
#define COLS_MAX 30         /* Maximum of 30 columns per input line */

/* one tsv field, stored within a fixed-length character string */
typedef char field_t[MAXFIELDLEN+1];


/**************************************************************/

int  mygetchar(void);
int  getfield(field_t dest);
int stage1(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int *row_total,
            int *col_total);
int stage2(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],int row, 
            int col, int integer[COLS_MAX], int len);
int stage3(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int integer[],
            int len, int row_total, int col_total);
void nums_of_rows_cols(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                    int *row_total, int *col_total);
void print_row(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int row, 
                int col);
void cols_for_sorted(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                    int integer[COLS_MAX], int len);
void insertion_sort(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
        int row, int col, int integer[COLS_MAX], int len);
void swap_2_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int row1,
                int row2);
void get_input_cols(int integer[COLS_MAX], int *len, int argc,
                    char *argv[]);
void print_first_middle_last_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                        int row, int col);
void horizontal_line(int total_len_row);
int find_max_len(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                    int row, int col, int integer[], int len);
void header_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                    int max_len, int integer[], int len);
void data_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
            int row, int integer[], int len, int max_len);
int check_row(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int i, 
                int integer[], int len, int max_len, int count);
void print_after_diff_cols(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
        int i, int integer[], int len, int max_len, int count, int col_diff);
void store_index_diff(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int i,
                int integer[], int len, int *col_diff);

/**************************************************************/

/* main program provides traffic control
*/
int
main(int argc, char *argv[]) {
    char array_for_input[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1];
    int row_total=0, col_total=0;

    stage1(array_for_input, &row_total, &col_total);

    printf("\n");

    int integer[COLS_MAX] = {0}, len = 0;
    get_input_cols(integer, &len, argc, argv);

    if (len) {
        stage2(array_for_input, row_total, col_total, integer, len);
        printf("\n");
        stage3(array_for_input, integer, len, row_total, col_total);
    }

	/* all done, time to go home */
    printf("ta daa!\n");
    return 0;
}

/**************************************************************/

/* read characters and build a string, stopping when a tab or newline
   as encountered, with the return value indicating what that
   terminating character was
*/
int
getfield(field_t dest) {

	int ch, nchs=0;
	int status=STATUS_NML;

	dest[0] = '\0';
	while ((ch=mygetchar())
		&& (ch != CHAR_TB) && (ch != CHAR_NL) && (ch != EOF)) {

		if (nchs < MAXFIELDLEN) {
			/* ok to save this character */
			dest[nchs++] = ch;
			dest[nchs] = '\0';
		} else {
			/* silently discard extra characters if present */
		}
	}

	/* return status is defined by what character stopped the loop */
	if (ch == EOF) {
		status = STATUS_EOF;
	} else if (ch == CHAR_NL) {
		status = STATUS_EOL;
	} else if (ch == CHAR_TB) {
		status = STATUS_NML;
	}
	return status;
}

/**************************************************************/

/* read a single character, bypassing any CR characters encountered,
   so as to work correctly with either PC-type or Unix-type input
 */
int
mygetchar(void) {
	int c;
	while ((c=getchar()) == CHAR_CR) {
		/* empty loop body */
	}
	return c;
}

/**************************************************************/

/* stage 1:
** 1. print out the number of rows for data and columns in the input.
** 2. Print the corresponding information for last row of input.
*/

int
stage1(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
        int *row_total, int *col_total) {

    printf("Stage 1\n");
    nums_of_rows_cols(arr, row_total, col_total);

    /* print the data of columns for last row */
    print_row(arr, *row_total, *col_total);
    return 0;
}


/*******************************************************************/

/* find, store and print out the nums of rows and cols in the input. */
void
nums_of_rows_cols(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                    int *row_total, int *col_total) {
    int rows=0, cols=0;
    field_t field;
    int status;

    while ((status = getfield(field)) != STATUS_EOF) {
        /* add field to array every time a new field is encountered */
        strcpy(arr[rows][cols], field);
        cols += 1;

        /* update cols_total for future use */
        if (cols > *col_total) {
            *col_total = cols;
        }

        /* move to next row if detected '\n' */
        if (status == STATUS_EOL) {
            rows += 1;
            cols = 0;
        }
    }

    /* update total row (cols already updated) 
    ** to be used in other functions 
    */
    *row_total = rows-1;  // -1 because we don't count the first row.

    printf("input tsv data has %d rows and %d columns\n", 
            *row_total, *col_total);
}

/*****************************************************************/

/* Print the data for specified columns of row */
void
print_row(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                int row, int col) {
    printf("row %d is:\n", row);

    for (int i = 0; i < col; i++) {
        printf("%4d: %-10s %s\n", i + 1, arr[0][i], arr[row][i]);
    }
}


/*******************************************************************/

/* stage 2:
** 1. sort the array of input data by the specified column.
** 2. Print the corresponding information .
*/

int
stage2(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
        int row, int col, int integer[COLS_MAX], int len) {
    
    printf("Stage 2\n");
    // Print out the columns we want to use for sorting.
    cols_for_sorted(arr, integer, len);

    // sort the array of input data by the input column.
    insertion_sort(arr, row, col, integer, len);

    // print out the first, middle, and last rows of sorted data.
    print_first_middle_last_rows(arr, row, col);
    return 0;
}

/*******************************************************************/

/* print out the columns we want to use for sorting 
** by iterating through the integers array.
*/

void
cols_for_sorted(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                int integer[COLS_MAX], int len) {

    printf("sorting by \"%s\",\n", arr[0][integer[0]-1]);

    for (int i = 1; i < len; i++) {

        if (i == len-1) {
            /* there is no comma after the last column */
            printf("   then by \"%s\"\n", arr[0][integer[i]-1]);
        } else {
            printf("   then by \"%s\",\n", arr[0][integer[i]-1]);
        }
    }
}


/*******************************************************************/

void
insertion_sort(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                            int row, int col, int integer[COLS_MAX],
                            int len) {

    /* sort the array of input data by the input column */
    for (int row_temp = 2; row_temp <= row; row_temp++) {
        int j = row_temp, sorted = 0;

        while (j > 1){
            int cmp_equal_times = 0;  // count the number of equal pairs.

            for (int index = 0; index < len; index++) {
                int index_col = integer[index] - 1;
                int cmp = strcmp(arr[j - 1][index_col], arr[j][index_col]);

                if (cmp > 0) {
                    swap_2_rows(arr, j - 1, j);
                    j--;
                    break;
                } else if (cmp < 0) {
                    sorted = 1;
                    break;
                } else {
                    cmp_equal_times += 1;
                }
            }

            if (sorted || cmp_equal_times == len) {
                /* Stop if the current pair is in correct order,
                ** or two rows are equal in all input columns. 
                */
                break;
            }
        }
    }
}

/************************************************************************/

void
swap_2_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                int row1, int row2) {
    char temp[MAXFIELDLEN + 1]; // create a temporary string to store the data.

    for (int i = 0; i < COLS_MAX; i++) {
        strcpy(temp, arr[row1][i]);  // store the data of row1 in temp.
        strcpy(arr[row1][i], arr[row2][i]);  // swap the data of row1 with row2.
        strcpy(arr[row2][i], temp);  // swap the data of row2 with temp.
    }
}

/********************************************************************/

/* get the input column for sorting */
void
get_input_cols(int integer[COLS_MAX], int *len, int argc, char *argv[]) {
    /* Store numbers of command-line arguments in integer array */
    for (int i = 1; i < argc; i++) {

        /* check if the input integer is valid */
        if (atoi(argv[i]) > 0 && atoi(argv[i]) < COLS_MAX) {
            /* convert the input integer to an index 
            ** and store it in integer array 
            */
            integer[i-1] = atoi(argv[i]);
            (*len) += 1;

        } else break; // break if no input integer is valid.
    }
}

/*******************************************************************/

/* print out the first, middle, and last rows of sorted data. */
void
print_first_middle_last_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1],
                            int row, int col) {

    if (row % 2 != 0) {
        /* if the number of rows is odd, 
        ** we should add to 1 to get the middle row.
        */
        row += 1;
        print_row(arr, 1, col);
        print_row(arr, row/2, col);
        print_row(arr, row-1, col);
    } else {
        print_row(arr, 1, col);
        print_row(arr, row/2, col);
        print_row(arr, row, col);
    }
}


/**************************************************************************/

/* stage 3:
** Generate a report that shows the 
** counts of rows matching the same selected column combination.
*/
int
stage3(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int integer[],
        int len, int row, int col) {

    printf("Stage 3\n");

    /* To find maximum lenth of row for first column */
    int max_len = find_max_len(arr, row, col, integer, len);

    /* then print out the header rows */
    int total_len_row = max_len + 6; // lenth of 'count' plus 1 blank space

    horizontal_line(total_len_row);
    header_rows(arr, max_len, integer, len);
    horizontal_line(total_len_row);

    /* print out the data rows */
    data_rows(arr, row, integer, len, max_len);
    horizontal_line(total_len_row);

    return 0;

}

/*************************************************************************/

void
horizontal_line(int total_len_row) {

    for (int i = 0; i < total_len_row; i++) {
        printf("-");
    }
    printf("\n");
}
/*************************************************************************/

int
find_max_len(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], 
                int row, int col, int integer[], int len) {
    int max_len = 0;
    int last_col_longest = integer[len-1]; // from input column
    
    /* find the maximum length of word according 
    ** to last input column of all rows.
    */
    for (int i = 0; i <= row; i++) {
        int len_str = (int)strlen(arr[i][last_col_longest-1]);

        if (len_str > max_len) {
            max_len = len_str;
        }
    }

    /* when number of command line integer is more than 1, next
    ** line should start at 5th letter of the previous row of word.
    ** (which means increase the length of row by 4)
    */
    if (len > 1) {
        max_len += (len-1) * 4;
    }

    return max_len;
}


/**************************************************************************/

/* To print out the header rows of the report. */
void
header_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int max_len,
                int integer[], int len) {

    for (int i = 0; i < len; i++) {

        /* Print the column headers */
        printf("%*s", i * 4, ""); // 4 blanks for new row.

        /* print the left_justified column header after 4 blanks */
        if (i == len-1) {
            printf("%-*s %5s\n", max_len - i * 4, arr[0][integer[i]-1],
                    "Count");
        } else {
            // to reduce the rest of blank space between max_len and string.
            int len_diff = max_len - (int)strlen(arr[0][integer[i]-1]);

            printf("%-*s\n", max_len - i * 4 - len_diff, arr[0][integer[i]-1]);
        }
    }
}

/**********************************************************************/

/* To print out the data rows of the report. */

void
data_rows(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int row,
            int integer[], int len, int max_len) {

    int col_diff = 0;
    /* pass to print out the rows that
    ** have the same input column of next row.
    */
    int pass = 0;

    /* to count the number of rows matching */
    int count = 1;
    for (int i = 1; i <= row; i++) {

        /* if the end of the input is reached...*/
        if (i == row) {
            if (pass) {
                /* To print out the column 
                ** of input after the col_diff
                */
                print_after_diff_cols(arr, i, integer, len,
                max_len, count, col_diff);
                pass = 0;
            } else {
                /* print out all input columns and its count 
                ** now col_diff is 0 already. 
                */
                print_after_diff_cols(arr, i, integer, len, max_len, 
                                    count, col_diff);
            }
            break;
        }

        int same_as_required = check_row(arr, i,
        integer, len, max_len, count);

        /* if all elements in integer are equal, then increment count,
        ** if not, print out and reset the count, then move to next row.
        */
        if (same_as_required) {
            count++;
        } else {
            if (pass) {
                /* To print out column of input after different intput 
                ** columns if have a 'pass' given from last row.
                */
                print_after_diff_cols(arr, i, integer, len, max_len, 
                                    count, col_diff);
                pass = 0;
            } else {
                /* To print out the current row of all (col_diff = 0)
                ** input columns and its count 
                */
                print_after_diff_cols(arr, i, integer, len, max_len, 
                                    count, col_diff);
            }

            /* record the index of the first column that 
            ** started to be different 
            */
            store_index_diff(arr, i, integer, len, &col_diff);
            if (col_diff != 0) {
                /* if a column is different, then give 
                ** a 'pass' to next row 
                */
                pass = 1;
            }
            count = 1; // Reset count for the next row
        }
    }
}

/**********************************************************************/

/* To print out column of input after the col_diff */
void print_after_diff_cols(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int i,
            int integer[], int len, int max_len, int count, int col_diff) {

    for (int j = col_diff; j < len; j++) {
        /* 4 blanks for new row */
        printf("%*s", j * 4, "");

        /* print the left_justified column data after 4 blanks */
        if (j == len-1) {
            printf("%-*s %5d\n", max_len - j * 4, arr[i][integer[j]-1], count);
        } else {
            // to reduce the rest of blank space between max_len and string.
            int len_diff = max_len - (int)strlen(arr[i][integer[j]-1]);

            printf("%-*s\n", max_len - j * 4 - len_diff, arr[i][integer[j]-1]);
        }
    }
}

/**************************************************************************/

/* check if the current row matches every column of input of next row */
int
check_row(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int i,
    int integer[], int len, int max_len, int count) {

    for (int j = 0; j < len; j++) {
        char *current_col = arr[i][integer[j] - 1];
        char *next_col = arr[i + 1][integer[j] - 1];

        // if not equal, then change the flag to 0.
        if (strcmp(current_col, next_col) != 0) {
            return 0;
        }
    }
    return 1;
}

/************************************************************/

/* This function is adapted from the check_row function.(07/09/24)
** Since if merge two functions, there will be a logical error.
** To store the index of the first column that started to be different.
*/
void
store_index_diff(char arr[LINE_MAX][COLS_MAX][MAXFIELDLEN + 1], int i,
    int integer[], int len, int *col_diff) {

    for (int j = 0; j < len; j++) {
        char *current_col = arr[i][integer[j] - 1];
        char *next_col = arr[i + 1][integer[j] - 1];

        if (strcmp(current_col, next_col) != 0) {
            /* record the index of the first column 
            ** that started to be different
            */
            *col_diff = j;
            break;
        }
    }
}

/* Algorithm is fun !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/