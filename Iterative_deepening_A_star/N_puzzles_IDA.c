/* 
	https://www.cs.princeton.edu/courses/archive/spr08/cos226/assignments/8puzzle.html	
	(N^2-1) puzzle using Iterative deepening A-star (IDA*) algorithm, as described here: 
	https://algorithmsinsight.wordpress.com/graph-theory-2/ida-star-algorithm-in-general/
	The Manhattan distance is used as the heuristic estimate.
	Heuristic calculation has been relatively optimized (updating after every move).
	Input: "start.txt" and "goal.txt". 
	Output: The solution printed to screen
	To compile with gcc, use:
	gcc -ansi -pedantic -W -Wall -o N_puzzles_IDA N_puzzles_IDA.c -pg
	Then run:
	./N_puzzles_IDA
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NUM_OF_POSSIBLE_MOVES 4 /* Up, down, left, right */
#define MAX_SOLUTION_LENGTH 1000
#define MAX_F_VALUE 100

int print_array(const int *arr, int N);

int** new_board(int N);

int print_board(const int** board, int N);

int free_board(int** board, int N);

int valid_moves(int N, int* result, int x_row, int x_col);

int print_solution(const int **start, int N, const char *desc);
	
int search(int **board, const int **goal, int N, int depth, int threshold, int *found, char *desc,
	char **solution, int *current_rows, int *current_cols, const int *goal_rows, const int * goal_cols, int N_squared, 
	int x_row, int x_col, int h_score);
	
int run(int **start, const int **goal, int N);

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
   
   
   run(start, (const int **) goal, N);    
    
   free_board(start, N);   
   free_board(goal, N);    
   return 0;
}

int print_array(const int *arr, int N){
	int i;
	for (i = 0; i < N; i++){
		printf("%d ", arr[i]);
	}
	printf("\n");
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
int print_board(const int** board, int N){
	int i, j;	
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			if (board[i][j] == 0) {
				printf("%2c ",'x');
			} else {
				printf("%2d ", board[i][j]);
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
		print_board((const int **) board, N);
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

int search(int **board, const int **goal, int N, int depth, int threshold, int *found, char *desc,
	char **solution, int *current_rows, int *current_cols, const int *goal_rows, const int * goal_cols, int N_squared, 
	int x_row, int x_col, int h_score){
		
	int f_score;
	int min, temp;
	int solution_length;
	int i;
	char move, go_back_move, last_move_by_current;	
	int old_x_row, old_x_col, new_x_row, new_x_col;
	int new_h_score;
	int temp_1, goal_row_temp, goal_col_temp;
	int N_minus_one = N - 1;
	
	/* printf("in search \n");
	*/
	
	f_score = depth + h_score;	
	if (f_score > threshold) {
		return f_score;
	}	
	if (h_score == 0){		
		*found = 1;		
		solution_length = 0;
		while (desc[solution_length] != '\0'){
			solution_length++;
		}
		*solution = (char *) malloc(sizeof(char) * solution_length);
		for (i = 0; i < solution_length; i++){
			(*solution)[i] = desc[i];
		}
		(*solution)[solution_length] = '\0';
		return f_score;		
	}
	min = INT_MAX;
	last_move_by_current = desc[depth]; /* desc always starts with 'B' so this is okay */	
	old_x_row = x_row;
	old_x_col = x_col;
	for (i = 0; i < NUM_OF_POSSIBLE_MOVES; i++){		
		switch (i) {
			case 0:
				move = 'u';
				go_back_move = 'd';
				new_x_row = x_row - 1;
				new_x_col = x_col;
				break;
			case 1:
				move = 'd';
				go_back_move = 'u';
				new_x_row = x_row + 1;
				new_x_col = x_col;
				break;
			case 2:
				move = 'l';
				go_back_move = 'r';
				new_x_row = x_row;
				new_x_col = x_col - 1;
				break;
			case 3:
				move = 'r';
				go_back_move = 'l';
				new_x_row = x_row;
				new_x_col = x_col + 1;
				break;
		}	
		if (move == 'u'){
			if (x_row == 0) {
				continue;
			}
		}
		if (move == 'd'){
			if (x_row == N_minus_one){
				continue;
			}
		}
		if (move == 'l'){
			if (x_col == 0){
				continue;
			}
		}
		if (move == 'r'){
			if (x_col == N_minus_one){
				continue;
			}
		}		
		if (last_move_by_current == go_back_move){
			/* No need to consider going back to the previous state */			
			continue;			
		}				
		
		/* Update h_score */
		new_h_score = h_score;
		temp_1 = board[new_x_row][new_x_col];
		goal_row_temp = goal_rows[temp_1];
		goal_col_temp = goal_cols[temp_1];
		/*
		new_h_score -= abs(current_rows[temp_1] - goal_row_temp);
		new_h_score -= abs(current_cols[temp_1] - goal_col_temp);			
		new_h_score += abs(current_rows[0] - goal_row_temp);
		new_h_score += abs(current_cols[0] - goal_col_temp);
		*/
		switch (go_back_move){
			case 'u':
				if (goal_row_temp < current_rows[temp_1]){
					new_h_score--;
				} else {
					new_h_score++;
				}
				break;
			case 'd':
				if (goal_row_temp > current_rows[temp_1]){
					new_h_score--;
				} else {
					new_h_score++;
				}
				break;
			case 'l':
				if (goal_col_temp < current_cols[temp_1]){
					new_h_score--;
				} else {
					new_h_score++;
				}
				break;
			case 'r':
				if (goal_col_temp > current_cols[temp_1]){
					new_h_score--;
				} else {
					new_h_score++;
				}
				break;				
		}
		
		/* Move */
		current_rows[0] = new_x_row;
		current_cols[0] = new_x_col;
		current_rows[temp_1] = old_x_row;
		current_cols[temp_1] = old_x_col;		
		board[old_x_row][old_x_col] = temp_1;
		board[new_x_row][new_x_col] = 0;		
		desc[depth+1] = move;
		desc[depth+2] = '\0';
		
		/* Search further down the game tree */
		temp = search(board, goal, N, depth+1, threshold, found, desc, solution, current_rows, current_cols,
			goal_rows, goal_cols, N_squared, new_x_row, new_x_col, new_h_score);		
		
		/* Move back */
		board[old_x_row][old_x_col] = 0;
		board[new_x_row][new_x_col] = temp_1;		
		desc[depth+1] = '\0';		
		current_rows[0] = old_x_row;
		current_cols[0] = old_x_col;
		current_rows[temp_1] = new_x_row;
		current_cols[temp_1] = new_x_col;
		
		if (*found == 1){
			return temp;
		}
		
		if (temp < min){
			 /* find the minimum of all f_score greater than threshold encountered */
			min = temp;
		}
	}
	*found = 0;
	return min;  /* return the minimum f_score encountered greater than threshold */
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
int run(int **start, const int **goal, int N){
	int threshold;
	int found = 0;	
	int *current_cols, *current_rows, *goal_cols, *goal_rows; /* Position of each element */
	int N_squared;
	int temp;
	char *desc = (char *) malloc(sizeof(char) * MAX_SOLUTION_LENGTH);
	char *solution = NULL;
	int i,j;
	int x_row, x_col;
	int h_score;	
	
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
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			current_rows[start[i][j]] = i;
			current_cols[start[i][j]] = j;
		}		
	}	
	
	/* Find 'x' position */
	for (i = 0; i < N; i++){		
		for (j = 0; j < N; j++){
			if (start[i][j] == 0) {
				x_row = i;
				x_col = j;				
			} 			
		}		
	}
	
	desc[0] = 'B';
	desc[1] = '\0';
	/* Here we use Manhattan score for the heuristic estimate */
	h_score = 0;
	for (i = 1; i < N_squared; i++){
	   /* Do not care about 'x' */
	   h_score += (abs(current_rows[i] - goal_rows[i]) + abs(current_cols[i] - goal_cols[i]));
    }
	threshold = h_score;	
	while (1){			
		temp = search(start, goal, N, 0, threshold, &found, desc, &solution, 
			current_rows, current_cols, (const int *)goal_rows, (const int *) goal_cols, N_squared, 
			x_row, x_col, h_score);
		if (found == 1){
			printf("FOUND SOLUTION!\n");
			print_solution((const int **) start, N, (const char *) solution);
			goto CLEANUP;				
		}
		if (temp > MAX_F_VALUE){
			/* Threshold larger than maximum possible f value */
			printf("MAXIMUM F VALUE REACHED! TERMINATING! \n");
			goto CLEANUP;
		}
		threshold = temp;
	}		
	CLEANUP: {
		free(solution);
		free(current_cols);
		free(current_rows);
		free(goal_cols);
		free(goal_rows);
		free(desc);
		return 0;
	}	
}
