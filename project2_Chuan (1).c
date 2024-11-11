/* 
Author by Yuchuan Zou for the Assignment 2 in COMP10002 Foundation-Of-Algorithm 
2024 S2 in the University of Melbourne.
uploading time: 11/11/2024.
The score is 20/20.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define SDELIM "==STAGE %d============================\n"   // stage delimiter
#define MDELIM "-------------------------------------\n"    // delimiter of -'s
#define THEEND "==THE END============================\n"    // end message

#define CRTRNC '\r'     // carriage return character
#define NBRHDS 8        // number of possible neighborhoods
#define RULE_1 184      // rule 184
#define RULE_2 232      // rule 232
#define HALF 0.5        // by comparing 1/2 to determine the result of stage 2;

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef char cells_t;            // base type to store states of cells
typedef struct state state_t;           // a cellular automaton state
typedef unsigned char rule_t[NBRHDS];   // an elementary CA update rule function

struct state {                   // a state in a CA is defined by
    cells_t*        clls;        // ... an array of cells and
    state_t*        next;        // ... a link to the next state
};

typedef struct {                 // a run of a CA consists of
    state_t*        init;        // ... the initial state and
    state_t*        curr;        // ... the current state,
} run_t;                         // implemented as a linked list of states


typedef struct {                 // an elementary CA is defined by
    unsigned char   code;        // ... a code of the update rule,
    unsigned int    size;        // ... a number of cells,
    unsigned int    time;        // ... the current time step,
    rule_t          rule;        // ... an update rule function, and
    run_t*          run;         // ... a run of state steps
} CA_t;

/* FUNCTIONS FOR WORKING WITH LINKED LISTS -----------------------------------*/
run_t*          makeemptylist(void);                   // make an empty list
void            freelist(run_t*);                      // free list

/* USEFUL FUNCTIONS ----------------------------------------------------------*/
int             mygetchar(void);                // getchar() that skips
                                                //    carriage returns

/* WORKING FUNCTIONS ---------------------------------------------------------*/
int             stage0(CA_t *ca_1);
void            read_input(CA_t *ca_1);
void            compute_store_rule(CA_t *ca_1, unsigned int code);
void            print_rule_binary(CA_t *ca_1);
int             stage1(CA_t *ca_1);
void            compute_next_state(CA_t *ca_1, state_t *start_state, 
                                    unsigned int time_step);
int             neighbor_code(CA_t *ca_1, int index, cells_t *cells_array);
void            print_required_state(state_t *start, int origin_steps, 
                                    int time_steps, int size);
cells_t*        allocate_cells(int size);
void            apply_update_rule(CA_t *ca_1, cells_t *curr_cells_array,
                                    cells_t *next_cells_array);
state_t*        update_linked_state(CA_t *ca_1, state_t *linked_state, 
                                    cells_t *next_cells_array);
void            count_on_off(CA_t *ca_1, int cell, int start, 
                                   int *on, int *off);
state_t*        locate_start_state(CA_t *ca_1, int start);
int             stage2(CA_t *ca_1, int size);
void            process_rule(CA_t *ca, int rule, int time_steps);
void            print_answer(int size, int start,double on);


/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[]) {
    CA_t *ca_1 = malloc(sizeof(CA_t));
    assert(ca_1 != NULL);
    ca_1->run = makeemptylist();
    assert(ca_1->run != NULL);

    stage0(ca_1);
    stage1(ca_1);
    stage2(ca_1, ca_1->size);

    // free the memory allocated for the CA structure
    freelist(ca_1->run);
    free(ca_1);
    return EXIT_SUCCESS;        // algorithms are fun!!!
}

/* USEFUL FUNCTIONS ----------------------------------------------------------*/

// An improved version of getchar(); skips carriage return characters
// and line feed characters to read useful input.
// NB: Adapted version of the mygetchar() function by Alistair Moffat
int mygetchar() {
    int c;
    while ((c=getchar())==CRTRNC   // skip carriage return characters
            || c == '\n');         // skip line feed characters
    return c;
}

/* Functions for linked list -------------------------------------------------*/

// make an empty linked list; adapted version of the make_empty_list function by
// Alistair Moffat: https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
run_t* makeemptylist(void) {
    run_t *run = (run_t*)malloc(sizeof(*run));
    assert(run!=NULL);
    run->init=run->curr=NULL;
    return run;
}

// free the input linked list; adapted version of the free_list function by
// Alistair Moffat: https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
void freelist(run_t *run) {
    assert(run != NULL);
    state_t *curr = run->init, *prev;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev->clls);
        free(prev);
    }
    free(run);
}

/*----------------------------------------------------------------------------*/

// stage 0: read the initial state of the CA and the size and rule of the CA.
int stage0(CA_t *ca_1) {
    read_input(ca_1);

    printf(SDELIM, 0);                     // 0 means stage 0
    printf("SIZE: %u\n", ca_1->size);
    printf("RULE: %u\n", ca_1->code);
    printf("%s", MDELIM);

    // to show the binary representation of the rule code
    compute_store_rule(ca_1, ca_1->code);
    print_rule_binary(ca_1);

    // to show the initial automaton state of the CA
    int initial_time_steps = 0;
    print_required_state(ca_1->run->init, initial_time_steps, 
                        initial_time_steps, ca_1->size);

    return 0;
}

/*----------------------------------------------------------------------------*/

// print the binary representation of the rule code.
void print_rule_binary(CA_t *ca_1) {
    printf(" 000 001 010 011 100 101 110 111\n");

    // print the binary representation of the rule code(reverse order).
    printf(" ");
    for (int i = 0; i < NBRHDS; i++) {
        if (i != NBRHDS - 1)  // if not the last code..
            printf(" %d  ", ca_1->rule[i]);
        else
            printf(" %d \n", ca_1->rule[i]);
    }
    printf("%s", MDELIM);
}

/*----------------------------------------------------------------------------*/
// to compute and store the rule function.
void compute_store_rule(CA_t *ca_1, unsigned int code) {
    for (int i = 0; i < NBRHDS; i++) {
        // shift bits to get the rightmost bit and store the update rule
        ca_1->rule[i] = (code >> i) & 1;
    }
}

/*----------------------------------------------------------------------------*/

// store the first 3 input(size, rule, and initial state) 
// and create an initial automaton of cells.
void read_input(CA_t *ca_1) {
    scanf("%u", &ca_1->size);
    scanf("%hhu", &ca_1->code);

    // allocate memory for new automaton of cells
    state_t *initial_state = (state_t*)malloc(sizeof(state_t));
    assert(initial_state != NULL);

    // allocate memory for the cells array with the size from the input
    initial_state->clls = allocate_cells(ca_1->size);

    // initialize the next cells array with NULL.
    initial_state->next = NULL;

    // read all the cells from the input and store them in the cells array.
    for (int i = 0; i < ca_1->size; i++) {
        initial_state->clls[i] = mygetchar();
    }

    // point the current and initial state to the initial_state at first.
    ca_1->run->init = ca_1->run->curr = initial_state;
}

/*----------------------------------------------------------------------------*/

// stage 1: compute and show all required state of the CA
// and count the on and off state according to the given position.
int stage1(CA_t *ca_1) {
    printf(SDELIM, 1);                     // 1 means stage 1

    // compute all required states of the CA.
    unsigned int time_steps;
    scanf("%d", &time_steps);
    ca_1->time = time_steps;               // update the time steps
    compute_next_state(ca_1, ca_1->run->init, time_steps);

    // print the required time steps of the CA.
    // 0 means starting from the initial state.
    print_required_state(ca_1->run->init, 0, ca_1->time, ca_1->size);
    printf("%s", MDELIM);

    // count and print the number of on and off state
    // according to the given position.
    int cell, start;        // to store the cell and start position (of row)
    scanf("%d, %d", &cell, &start);

    int on = 0, off = 0;    // to store the number of on and off state
    count_on_off(ca_1, cell, start, &on, &off);
    printf("#ON=%d #OFF=%d CELL#%d START@%d\n", on, off, cell, start);
    return 0;
}

/*----------------------------------------------------------------------------*/

// to compute the next state of the CA and store it in the linked list
void compute_next_state(CA_t *ca_1, state_t *start_state,
                            unsigned int time_steps) {
    // linked_state is the state that is ready to link with the next state.
    state_t *linked_state = start_state;  // start from the required state
    cells_t *curr_cells_array = linked_state->clls;

    // compute and store all required number of states of the CA.
    // add new stated after the last_time_step.
    for (int i = 1; i <= time_steps; i++) {
        cells_t *next_cells_array = allocate_cells(ca_1->size);

        // compute the next state based on the current state and the update rule
        apply_update_rule(ca_1, curr_cells_array, next_cells_array);

        // link the next state to the current one and update pointers
        linked_state = update_linked_state(ca_1, linked_state, 
                                            next_cells_array);

        // update current state for next iteration
        curr_cells_array = next_cells_array;
    }

}

/*----------------------------------------------------------------------------*/
// to allocate memory for the cells array
cells_t* allocate_cells(int size) {
    cells_t *cells = (cells_t*)malloc(size * sizeof(cells_t));
    assert(cells != NULL);
    return cells;
}

/*----------------------------------------------------------------------------*/

// Function to apply the update rule and compute the next state of the CA
void apply_update_rule(CA_t *ca_1, cells_t *curr_cells_array, 
                        cells_t *next_cells_array) {
    for (int j = 0; j < ca_1->size; j++) {
        // find the index of the 'rule function' to apply for the given position
        int index = neighbor_code(ca_1, j, curr_cells_array);

        // apply the rule to compute the next state of the cell
        if (ca_1->rule[index] == 1) {
            next_cells_array[j] = '*';
        } else {
            next_cells_array[j] = '.';
        }
    }
}

/*----------------------------------------------------------------------------*/

// to update the linked state and current pointers
state_t *update_linked_state(CA_t *ca_1, state_t *linked_state, 
                            cells_t *next_cells_array) {
    linked_state->next = (state_t*)malloc(sizeof(state_t));
    assert(linked_state->next != NULL);
    // update the cells array of the next state
    linked_state->next->clls = next_cells_array;
    linked_state->next->next = NULL;

    ca_1->run->curr = linked_state->next;  // update current state pointer
    return linked_state->next;             // return updated linked state
}

/*----------------------------------------------------------------------------*/

// to find the index of the code of the rule function 
// for the same position j in next cells_array.
int neighbor_code(CA_t *ca_1, int index, cells_t *cells_array) {
    char *neighbor_comb[8] = {"...", "..*", ".*.", ".**", 
                            "*..", "*.*", "**.", "***"};
    char comb_index[4];

    if (index == 0) {
        // left neighbor is the last cell of the array
        comb_index[0] = cells_array[ca_1->size - 1];
        comb_index[1] = cells_array[index];
        comb_index[2] = cells_array[index + 1];
    } else if (index == ca_1->size - 1) {
        comb_index[0] = cells_array[index - 1];
        comb_index[1] = cells_array[index];
        // right neighbor is the first cell
        comb_index[2] = cells_array[0];
    } else {
        comb_index[0] = cells_array[index - 1];
        comb_index[1] = cells_array[index];
        comb_index[2] = cells_array[index + 1];
    }
    comb_index[3] = '\0';

    // match the combination of the neighbor cells with the possible
    // combinations to get the index of code (1 or 0) of the rule.
    for (int i = 0; i < NBRHDS; i++) {
        if (strcmp(neighbor_comb[i], comb_index) == 0) {
            return i;
        }
    }

    return -1;  // it should never reach here.
}

/*----------------------------------------------------------------------------*/

// To print the required number of states of the CA in the input.
void print_required_state(state_t *start, int origin_steps,
                            int time_steps, int size) {
    state_t *curr_state = start;     // start from the required start state

    for (int i = origin_steps; i <= origin_steps + time_steps; i++) {
        printf("%4d: ", i);
        // move to the next state if not initial or last state.
        if (i > origin_steps) {
            curr_state = curr_state->next;
        }
        for (int j = 0; j < size; j++) {
            printf("%c", curr_state->clls[j]);
        }
        printf("\n");
    }
}

/*----------------------------------------------------------------------------*/

// To count the number of on and off state according to the given position.
void count_on_off(CA_t *ca_1, int cell, int start, int *on, int *off) {
    state_t *start_state = locate_start_state(ca_1, start);

    for (int i = start; i <= ca_1->time; i++) {
        if (start_state->clls[cell] == '*') *on += 1;
        else *off += 1;

        // move to the next state if it is not the last state.
        if (i < ca_1->time) start_state = start_state->next;
    }
}

/*----------------------------------------------------------------------------*/

// To locate the start state of the CA based on the given start position.
state_t* locate_start_state(CA_t *ca_1, int start) {
    // start from the initial state
    state_t *start_state = ca_1->run->init;

    // move to the required start state
    for (int i = 0; i < start && start_state != NULL; i++) {
        start_state = start_state->next;
    }

    return start_state;
}

/*----------------------------------------------------------------------------*/

// stage2: apply the Fuk's procedure to solve the density classification problem
int stage2(CA_t *ca_1, int size) {
    int cell, start;
    scanf("%d, %d", &cell, &start);
    printf(SDELIM, 2);                    // 2 means stage 2
    // store the last time step of CA before updating the new time steps.
    int time_step_start = ca_1->time;
    state_t *start_state = ca_1->run->curr;

    // find the time steps to compute based on the cell count
    int time_step = (size - 2) / 2;
    printf("RULE: %d; STEPS: %d.\n%s", RULE_1, time_step, MDELIM);
    process_rule(ca_1, RULE_1, time_step);  // process rule 184

    // find the time steps for the second rule.
    time_step = (size - 1) / 2;
    printf("%sRULE: %d; STEPS: %d.\n%s", MDELIM, RULE_2, time_step, MDELIM);
    process_rule(ca_1, RULE_2, time_step);  // process rule 232

    int on = 0, off = 0;
    count_on_off(ca_1, cell, start, &on, &off);
    printf("%s#ON=%d #OFF=%d CELL#%d START@%d\n%s", MDELIM
                                , on, off, cell, start, MDELIM);

    // print and analyze the last cells array of state of the CA in stage 1.
    // 0 means print that one state only and don't print states after that.
    print_required_state(start_state, time_step_start, 
                        0, ca_1->size);

    double on_1 = 0.0;
    // compute the ratio of on cells in last automation in stage 1.
    for (int i = 0; i < ca_1->size; i++) {
        if (ca_1->run->curr->clls[i] == '*') on_1 += 1.0;
    }
    print_answer(size, time_step_start, on_1);
    printf("%s", THEEND);
    return 0;
}

/*----------------------------------------------------------------------------*/

// use the given rule to compute the next states and print the required state.
void process_rule(CA_t *ca, int rule, int time_steps) {
    // store the last state of CA before computing the next state
    state_t *start_state = ca->run->curr;

    // update the rule to apply
    compute_store_rule(ca, rule);

    // compute the next states required for the given time steps
    compute_next_state(ca, start_state, time_steps);

    // print out the required state
    print_required_state(start_state, ca->time, time_steps, ca->size);
    ca->time += time_steps;  // update the current time steps
}

/*----------------------------------------------------------------------------*/

// to print the answer based on the ratio of given on and cell count.
void print_answer(int size, int start, double on) {
    double result = on / (double)size;

    if (!result) printf("AT T=%d: #ON/#CELLS < 1/2\n", start);
    else if (result == HALF ) printf("AT T=%d: #ON/#CELLS = 1/2\n", start);
    else if (result == 1.0) printf("AT T=%d: #ON/#CELLS > 1/2\n", start);
}
/*----------------------------------------------------------------------------*/
