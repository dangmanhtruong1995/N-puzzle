/* 
	https://www.cs.princeton.edu/courses/archive/spr08/cos226/assignments/8puzzle.html
	Should have been (N^2 - 1) puzzle but whatever
	Anyway here we use the A star algorithm
	Input: "start.txt" and "goal.txt". 
	Output: The solution printed to screen
	To compile with gcc, use:
	gcc -ansi -pedantic -W -Wall -o N_puzzles N_puzzles.c
	Then run:
	./N_puzzles
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_MOVES 4 /* Up, down, left, right */
#define MAX_HEAP_SIZE 20

typedef struct StateStruct {
	char* desc; /* The list of moves to get to the board position. For example 'Brd' for right then down */
	int** board;
	int score;
	int g_score;	
} State;

int** new_board(int N);

int print_board(int** board, int N);

int free_board(int** board, int N);

int is_valid_move(int** board, int N, char move);

int valid_moves(int** board, int N, int* result);

int** move_board(int** board, int N, char move);

char* move_desc(const char* desc, char move);

int h_score(int** board, int *current_rows, int *current_cols, const int *goal_rows, const int *goal_cols,
	int N, int N_squared);

int swap_two_states(State *state_1, State *state_2);

int is_goal(const int **board, const int **goal, int N);

int print_solution(const int **start, int N, const char *desc);
	
int insert_to_heap(State *heap, int *heap_size, char *desc, int **board, int score, int g_score);

int extract_from_heap(State *heap, int *heap_size, char **desc, int ***board, int *score, int *g_score);
	
int run(const int** start, const int ** goal, int N);

int main()
{
   
   FILE *fid;
   int N, i, j;   
   int **start, **goal;     
  
   /* Read start position */
   fid = fopen("start.txt", "rt");
   fscanf(fid, "%d ", &N);   
   start = new_board(N);
   for (i = 0; i < N; i++){
	   for (j = 0; j < N; j++){
		   fscanf(fid, "%d ", &start[i][j]);
	   }
   }
   fclose(fid);
      
   /* Read goal position */
   fid = fopen("goal.txt", "rt");   
   goal = new_board(N);
   for (i = 0; i < N; i++){
	   for (j = 0; j < N; j++){
		   fscanf(fid, "%d ", &goal[i][j]);
	   }
   }
   fclose(fid);   
   
   run((const int**) start, (const int **) goal, N);    
   
   free_board(start, N);   
   free_board(goal, N);    
   return 0;
}

int** new_board(int N){
   int **board, i;
   board = (int **) malloc(sizeof(int *) * N);
   for (i = 0; i < N; i++){
	   board[i] = (int *) malloc(sizeof(int) * N);
   }
   return board;
}

/*
 * Function:  print_board 
 * --------------------
 * Print the board
 *    
 *  board: The board configuration 
 *  N: width and height of the board 
 * 
 *  returns: 0
 */
int print_board(int** board, int N){
	int i, j;	
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			if (board[i][j] == 0) {
				printf("x ");
			} else {
				printf("%d ", board[i][j]);
			}
		}
		printf("\n");
	}
	return 0;
}

/*
 * Function:  free_board 
 * --------------------
 * Free the board
 *    
 *  board: The board configuration 
 *  N: width and height of the board 
 * 
 *  returns: 0
 */
int free_board(int** board, int N){
	int i;
	for (i = 0; i < N; i++){
		free(board[i]);
	}
	free(board);
	return 0;
}

/*
 * Function:  is_valid_move 
 * --------------------
 * Check whether the move is valid or not
 *    
 *  board: The board configuration 
 *  N: width and height of the board 
 *  move: The move('u', 'd', 'l', 'r') 
 *
 *  returns: 1 if valid and 0 otherwise
 */
int is_valid_move(int** board, int N, char move){
	int i, j, x_row, x_col, new_x_row, new_x_col;
	
	/* Find 'x' position */
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			if (board[i][j] == 0) {
				x_row = i;
				x_col = j;
			} 
		}		
	}
	
	/* Then check if the new position is within the board */
	switch (move){
		case 'u':
			/* Up */
			new_x_row = x_row - 1;
			new_x_col = x_col;
			break;
		case 'd':
			/* Down */
			new_x_row = x_row + 1;
			new_x_col = x_col;
			break;
		case 'l':
			/* Left */
			new_x_row = x_row;
			new_x_col = x_col - 1;
			break;
		case 'r':
			/* Right */
			new_x_row = x_row;
			new_x_col = x_col + 1;
			break;
		default:
			new_x_row = -1;
			new_x_col = -1;
			break;
	}
	if ((0 <= new_x_row) && (new_x_row < N) && (0 <= new_x_col) && (new_x_col < N)){
		return 1;
	} else {
		return 0;
	}
}

/*
 * Function:  valid_moves 
 * --------------------
 * Return the list of valid moves from the current board
 *    
 *  board: The board configuration 
 *  N: width and height of the board 
 *  result: The array of size NUM_OF_MOVES used to store the list of valid moves
 *  result[i] = 1 if the move i is valid and 0 otherwise
 *  i = 0: Up
 *  i = 1: Down
 *  i = 2: Left
 *  i = 3: Right
 *
 *  returns: 0
 */
int valid_moves(int** board, int N, int* result){
	/* Up, down, left, right	 */
	result[0] = is_valid_move(board, N, 'u');
	result[1] = is_valid_move(board, N, 'd');
	result[2] = is_valid_move(board, N, 'l');
	result[3] = is_valid_move(board, N, 'r');
	return 0;
}

/*
 * Function:  move_board 
 * --------------------
 * Create a new board based on the designated move
 *    
 *  board: The board configuration 
 *  N: width and height of the board 
 *  move: The move('u', 'd', 'l', 'r') 
 *
 *  returns: a new dynamically created board (that must be freed afterwards)
 */
int** move_board(int** board, int N, char move){
	/* This function assumes that the move is valid */
	int **board_2 = new_board(N);
	int i, j, x_row, x_col, new_x_row, new_x_col;	
	/* Find 'x' position */
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			if (board[i][j] == 0) {
				x_row = i;
				x_col = j;
			} 
		}		
	}
	
	/* Then move! */
	switch (move){
		case 'u':
			/* Up */
			new_x_row = x_row - 1;
			new_x_col = x_col;
			break;
		case 'd':
			/* Down */
			new_x_row = x_row + 1;
			new_x_col = x_col;
			break;
		case 'l':
			/* Left */
			new_x_row = x_row;
			new_x_col = x_col - 1;
			break;
		case 'r':
			/* Right */
			new_x_row = x_row;
			new_x_col = x_col + 1;
			break;		
	}
	
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			board_2[i][j] = board[i][j];
		}		
	}
	board_2[x_row][x_col] = board[new_x_row][new_x_col];
	board_2[new_x_row][new_x_col] = board[x_row][x_col];
	return board_2;
}

/*
 * Function:  move_desc 
 * --------------------
 * Create a new move description (e.g: Brdrd) based on the designated move
 *    
 *  desc: The move description, basically a character array that starts with 'B' and ends with '\0'  
 *  move: The move('u', 'd', 'l', 'r') 
 *
 *  returns: a new dynamically created move description (that must be freed afterwards)
 */
char* move_desc(const char* desc, char move){
	int desc_length = strlen(desc);
	int i;
	char *desc_2 = (char *) malloc(sizeof(char) * (desc_length+1));
	for (i = 0; i < desc_length; i++){
		desc_2[i] = desc[i];
	}
	desc_2[desc_length] = move;
	desc_2[desc_length + 1] = '\0';
	return desc_2;
}

/*
 * Function:  h_score 
 * --------------------
 * Calculate the h_score (heuristic estimate) of the current board configuration
 * Here we use Manhattan distance as the heuristic estimate
 *    
 *  board: The board configuration
 *  current_rows: Row position for each of the value in the current board (current_rows[value] = position) 
 *  current_cols: Column position for each of the value in the current board (current_cols[value] = position) 
 *  goal_rows: Row position for each of the value in the goal (goal_rows[value] = position)
 *  goal_cols: Column position for each of the value in the goal (goal_cols[value] = position)
 *  N: width and height of the board 
 *  N_squared: This is because we will call h_score multiple times
 * 
 *  returns: 0
 */
int h_score(int** board, int *current_rows, int *current_cols, const int *goal_rows, const int *goal_cols,
	int N, int N_squared){
	/* Here we use Manhattan priority function */
	int i,j;
	int manhattan_score;
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			current_rows[board[i][j]] = i;
			current_cols[board[i][j]] = j;
		}		
	}
	for (i = 1; i < N_squared; i++){
	   /* Do not care about 'x' */
	   manhattan_score += (abs(current_rows[i] - goal_rows[i]) + abs(current_cols[i] - goal_cols[i]));
   }
   return manhattan_score;	
}

/*
 * Function:  swap_two_states 
 * --------------------
 * Swap two states
 *    
 *  state_1: The first state
 *  state_2: The second state 
 *
 *  returns: 0
 */
int swap_two_states(State *state_1, State *state_2){
	State temp;
	temp.desc = (*state_1).desc;
	temp.board = (*state_1).board;
	temp.score = (*state_1).score;
	temp.g_score = (*state_1).g_score;
	
	(*state_1).desc = (*state_2).desc;
	(*state_1).board = (*state_2).board;
	(*state_1).score = (*state_2).score;
	(*state_1).g_score = (*state_2).g_score;
	
	(*state_2).desc = temp.desc;
	(*state_2).board = temp.board;
	(*state_2).score = temp.score;
	(*state_2).g_score = temp.g_score;
	
	return 0;
}
/*
 * Function:  is_goal 
 * --------------------
 * Check if a board configuration is the goal state or not
 *    
 *  board: The board configuration
 *  goal: The goal state
 *  N: width and height of the board 
 * 
 *  returns: 1 if it is true else 0
 */
int is_goal(const int **board, const int **goal, int N){
	int i, j;	
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			if (board[i][j] != goal[i][j]){
				return 0;
			}
		}
	}
	return 1;
}

/*
 * Function:  print_solution 
 * --------------------
 * Print the optimal solution
 *    
 *  start: The start state *  
 *  N: width and height of the board 
 *  desc: A descriptions of the moves to get from the start state to the goal state'
 *  For example : "Brdrd" would be "right, down, right, down" 
 *  u: Up, d: Down, l: Left, r: Right. B denotes the starting position
 *
 *  returns: 0
 */
int print_solution(const int **start, int N, const char *desc){
	int **board, i, j, temp, x_row, x_col, new_x_row, new_x_col;	
	char move;
	board = new_board(N);
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			board[i][j] = start[i][j];	
			if (board[i][j] == 0) {
				x_row = i;
				x_col = j;
			} 			
		}		
	}
	printf("Moves to get to the solution: %s \n", desc);
	
	printf("The solution: \n");	
	i = 1;
	while (1) {
		print_board(board, N);
		printf(" => \n\n");		
		move = desc[i];
		if (move == '\0'){
			printf("The end!\n");
			for (i = 0; i < N; i++){
				free(board[i]);
			}
			free(board);
			return 0;
		}
		switch (move){
			case 'u':
				/* Up */
				new_x_row = x_row - 1;
				new_x_col = x_col;
				break;
			case 'd':
				/* Down */
				new_x_row = x_row + 1;
				new_x_col = x_col;
				break;
			case 'l':
				/* Left */
				new_x_row = x_row;
				new_x_col = x_col - 1;
				break;
			case 'r':
				/* Right */
				new_x_row = x_row;
				new_x_col = x_col + 1;
				break;		
		}
		temp = board[new_x_row][new_x_col];
		board[new_x_row][new_x_col] = board[x_row][x_col];
		board[x_row][x_col] = temp;	
		x_row = new_x_row;
		x_col = new_x_col;
		i++;
	}	
	return 0;
}
/*
 * Function:  insert_to_heap 
 * --------------------
 * insert an element to heap (note that the heap starts from the 1st index, not 0)
 *    
 *  heap: The heap, represented as an array
 *  heap_size: Current size of the heap
 *  desc: The description character array (created dynamically). It will be linked to the new heap element
 *  board: The board (created dynamically). It will be linked to the new heap element
 *  score: The f-score 
 *  g_score: The number of moves made to arrive at the current board position
 * 
 *  returns: 0
 */
int insert_to_heap(State *heap, int *heap_size, char *desc, int **board, int score, int g_score){
	
	/* Add the element to the bottom level of the heap */
	int child_idx, parent_idx;
	*heap_size = (*heap_size) + 1;
	heap[*heap_size].desc = desc;
	heap[*heap_size].board = board;
	heap[*heap_size].score = score;
	heap[*heap_size].g_score = g_score;
	
	if (*heap_size == 1){
		/* The heap has only 1 element so stop */
		return 0;
	}
	child_idx = *heap_size;
	parent_idx = child_idx / 2;
	while (parent_idx != 0) {
		/* Compare the added element with its parent; if they are in the correct order, stop. */
		if (heap[parent_idx].score <= heap[child_idx].score){
			return 0;
		} else {
			/* Swap the element with its parent and return to the previous step	*/
			swap_two_states(&heap[parent_idx], &heap[child_idx]);
			child_idx = parent_idx;
			parent_idx /= 2;
		}		
	}
	return 0;	
}

/*
 * Function:  extract_from_heap 
 * --------------------
 * Extract an element from the heap (note that the heap starts from the 1st index, not 0)
 *    
 *  heap: The heap, represented as an array
 *  heap_size: Current size of the heap
 *  desc: The description character array (output)
 *  board: The board (output)
 *  score: The f-score (output)
 *  g_score: The number of moves made to arrive at the current board position (output)
 * 
 *  returns: 0
 */
int extract_from_heap(State *heap, int *heap_size, char **desc, int ***board, int *score, int *g_score){
	int child_to_swap_with_idx, left_child_idx, right_child_idx, parent_idx;	
	if ((*heap_size) == 0){
		return 0;
	}
	/* Extract from root */	
	(*desc) = heap[1].desc;
	(*board) = heap[1].board;
	(*score) = heap[1].score;
	(*g_score) = heap[1].g_score;
	
	/* Replace the root of the heap with the last element on the last level */	
	heap[1].desc = heap[*heap_size].desc;
	heap[1].board = heap[*heap_size].board;
	heap[1].score = heap[*heap_size].score;
	heap[1].g_score = heap[*heap_size].g_score;
	heap[*heap_size].desc = NULL;
	heap[*heap_size].board = NULL;
	*heap_size = (*heap_size) - 1;	
	
	if ((*heap_size) == 1) {
		return 0;
	}
	
	parent_idx = 1;	
	while (1){
		/* Compare the parent with its children; if they are in the correct order, stop	*/		
		left_child_idx = 2*parent_idx ; 
		right_child_idx = 2*parent_idx + 1;
		child_to_swap_with_idx = parent_idx;
		if (left_child_idx <= *heap_size){
			if (heap[parent_idx].score > heap[left_child_idx].score){
				child_to_swap_with_idx = left_child_idx;
			}
		}
		if (right_child_idx <= *heap_size){
			if (heap[left_child_idx].score > heap[right_child_idx].score){
				child_to_swap_with_idx = right_child_idx;
			}
		}
		if (child_to_swap_with_idx == parent_idx){
			return 0;
		}
		/* If not, swap the parent with one of its children and return to the previous step. 
		   (Swap with its smaller child in a min-heap and its larger child in a max-heap)
		 */		 
		swap_two_states(&heap[parent_idx], &heap[child_to_swap_with_idx]);
		parent_idx = child_to_swap_with_idx;		
	}	
}

/*
 * Function:  run 
 * --------------------
 * Run (N^2-1) solver using A-star algorithm
 *    
 *  start: The starting board
 *  goal: The goal
 *  N: width and height of the board 
 * 
 *  returns: 0
 */
int run(const int** start, const int ** goal, int N){
	int *current_cols, *current_rows, *goal_cols, *goal_rows; /* Position of each element */
	int i,j;
	int N_squared;
	
	State heap[MAX_HEAP_SIZE];
	int heap_size = 0;
	char *desc, *desc_2;	
	int **board, **board_2;
	int score, score_2;
	int g_score, g_score_2;
	/* int *list_of_valid_moves = (int *) malloc(sizeof(int) * NUM_OF_MOVES);
	*/
	int list_of_valid_moves[NUM_OF_MOVES];
	int desc_length;
	char move, go_back_move, last_move_by_current;
	
	/* Precompute positions for goal */
	N_squared = N*N;
	current_cols = (int *) malloc(sizeof(int) * N_squared);
    current_rows = (int *) malloc(sizeof(int) * N_squared);
    goal_cols = (int *) malloc(sizeof(int) * N_squared);
    goal_rows = (int *) malloc(sizeof(int) * N_squared);
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			goal_rows[goal[i][j]] = i;
			goal_cols[goal[i][j]] = j;
		}		
	}
	
	/* First, insert the initial state into the heap */
	desc = (char *) malloc(sizeof(char) * N_squared);
	desc[0] = 'B';
	desc[1] = '\0';	
	board = new_board(N);
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			board[i][j] = start[i][j];			
		}		
	}
	g_score = 0;
	score = g_score + h_score(board, current_rows, current_cols, 
		(const int *) goal_rows, (const int *) goal_cols, N, N_squared);	
	insert_to_heap(heap, &heap_size, desc, board, score, g_score);	
	desc = NULL;
	board = NULL;
	
	while (1){
		/* Extract the state with the lowest score from the heap
		 */
		if (heap_size == 0){
			printf("The heap is empty already! Something must be wrong! \n");
			free(desc);
			for (i = 0; i < N; i++){
				free(board[i]);
			}			
			free(board);
			free(current_cols);
			free(current_rows);
			free(goal_cols);
			free(goal_rows);			
			for (i = 1; i <= heap_size; i++){
				free(heap[i].desc);
				for (j = 0; j < N; j++){
					free(heap[i].board[j]);
				}
				free(heap[i].board);
			}
			return 0;
		}
		if (desc != NULL){
			free(desc);
		}
		if (board != NULL){					
			for (i = 0; i < N; i++){
				free(board[i]);
			}
			free(board);
		}
		extract_from_heap(heap, &heap_size, &desc, &board, &score, &g_score);
		
		/* If it is the goal state then stop and print the solution (don't forget to cleanup the memory!)
		 */
		if (is_goal((const int **) board, goal, N)){
			print_solution(start, N, (const char *) desc);
			free(desc);
			for (i = 0; i < N; i++){
				free(board[i]);
			}
			free(board);
			free(current_cols);
			free(current_rows);
			free(goal_cols);
			free(goal_rows);
			for (i = 1; i <= heap_size; i++){
				free(heap[i].desc);
				for (j = 0; j < N; j++){
					free(heap[i].board[j]);
				}
				free(heap[i].board);
			}
			return 0;
		}
		
		/* Else insert all neighboring states (those that can be reached in one move 
		   from the extracted state) onto the heap
		 */
		desc_length = strlen(desc);
		last_move_by_current = desc[desc_length - 1]; /* desc always starts with 'B' so this is okay */			
		valid_moves(board, N, list_of_valid_moves);
		for (i = 0; i < NUM_OF_MOVES; i++){			
			if (list_of_valid_moves[i] == 0){
				continue;
			}
			switch (i) {
				case 0:
					move = 'u';
					break;
				case 1:
					move = 'd';
					break;
				case 2:
					move = 'l';
					break;
				case 3:
					move = 'r';
					break;
			}			
			switch (move) {
				case 'u':
					go_back_move = 'd';
					break;
				case 'd':
					go_back_move = 'u';
					break;
				case 'l':
					go_back_move = 'r';
					break;
				case 'r':
					go_back_move = 'l';
					break;
			}
			if (last_move_by_current == go_back_move){
				continue;			
			}
			board_2 = move_board(board, N, move);			
			desc_2 = move_desc(desc, move);
			g_score_2 = g_score + 1;
			score_2 = g_score_2 + h_score(board_2, current_rows, current_cols, (const int *)goal_rows, 
				(const int *) goal_cols, N, N_squared);			
			insert_to_heap(heap, &heap_size, desc_2, board_2, score_2, g_score_2);
			board_2 = NULL;
			desc_2 = NULL;			
		}
	}	
}
