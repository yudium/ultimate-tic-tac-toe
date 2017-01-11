#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define player1 0
#define player2 1
#define player1_symbol 'O'
#define player2_symbol 'X'

#define false 0
#define true 1

#define GREEN 2
#define BLUE 1
#define RED 4
#define WHITE 15

typedef int bool; // create boolean type
typedef struct {
    int current_player; // only for local game
    int **box; // 3x3 matrix
    int **map_box; // 9x9 matrix; only for global game
} st_game;
int smart_player;
int dump_player;

/* procedure & function declarations */
st_game create_new_state_of(st_game game, int move[2], bool removeBox);
int score(st_game game, int player, int depth);
bool isOver(int **box);
bool isDraw(int **box);
void appendLocalToGlobal(st_game global_game, st_game local_game, int turn);
int suit();
st_game initialize_game(st_game game, int player);
st_game initialize_global_game(st_game game, int player);
void createBoard();
void updateBoard(int **box, int turn);
void delete2DArray(int **arr, int height, int width);
int** create2DArray(int height, int width);
int** createCopy2DArray(int **arr_ori, int height, int width);
int** get_available_moves(int len, int **box);
int count_available_moves(int **box);
int get_max_element_of_array(int arr[], int n);
int get_min_element_of_array(int arr[], int n);
bool gotoxy(int x, int y);
void setColor(int color);

int main()
{
    setColor(WHITE);
    srand(time(NULL));

    // this deciding who are first player and who are smart/dump player
    int first_player;
    first_player=suit();

    // print out the current game player status
    gotoxy(45, 2); printf("|First player is %c|", first_player==player1? player1_symbol:player2_symbol);
    gotoxy(45, 3); printf("|Smart player is %c|", smart_player==player1? player1_symbol:player2_symbol);
    gotoxy(45, 4); printf("|Dump player  is %c|", dump_player==player1? player1_symbol:player2_symbol);
    
    // initialize global game
    st_game global_game;
    global_game=initialize_global_game(global_game, first_player);

    // initialize local game
    st_game game;
    game=initialize_game(game, first_player);

    // running the game
    int i=0, global_turn_number=0, move_global[2], move[2];
    do {
        i++;
        // brute-force strategy in global game for all player
        minimax(global_game, global_game.current_player, 1, move_global);
        
        if (move_global[0]==0) global_turn_number=move_global[1];
        if (move_global[0]==1) global_turn_number=3+move_global[1];
        if (move_global[0]==2) global_turn_number=6+move_global[1];

        // this is local game
        do {
            // brute-force strategy in local game for all player
            minimax(game, game.current_player, 1, move);

            // get new state of game and change current player
            game=create_new_state_of(game, move, true);
            appendLocalToGlobal(global_game, game, global_turn_number);
            updateBoard(global_game.map_box, global_turn_number);
            
            sleep(500);
        } while (isOver(game.box)==false && isDraw(game.box)==false);

        if (isOver(game.box)==true)
            global_game.box[move_global[0]][move_global[1]]=1-game.current_player;
        if (isDraw(game.box)==true)
            global_game.box[move_global[0]][move_global[1]]=3;

        global_game.current_player=game.current_player;

        // reset local game box
        delete2DArray(game.box, 3, 3);
        game=initialize_game(game, game.current_player);

    } while (isOver(global_game.box)==false && isDraw(global_game.box)==false);

    // print out status of the end of game
    gotoxy(45, 6);
    if (isOver(global_game.box)==true) {
        printf("PLAYER %c IS WIN!", 1-global_game.current_player==player1? player1_symbol:player2_symbol);
    } else {
        printf("THE GAME IS DRAW!");
    }

    gotoxy(45, 8); printf("Press Enter to Exit...");
    getch();
    delete2DArray(global_game.box, 3, 3);
    delete2DArray(global_game.map_box, 9, 9);
    return 0;
}

int minimax(st_game game, int player, int depth, int move[2])
{
    int max_depth;
    if (player==smart_player) max_depth=40;
    if (player==dump_player) max_depth=3;
    if (isOver(game.box)) return score(game, player, depth);
    if (depth==max_depth || isDraw(game.box)==true) return 0;

    int i, total_moves, **moves;
    st_game simulation_game;
    total_moves=count_available_moves(game.box);
    moves=get_available_moves(total_moves, game.box);

    // @var scores have same length with @var moves
    int* scores=(int*) malloc(total_moves * sizeof(int));
    // iterate all move, implement it to simulation game and check the score
    for (i=0; i<total_moves; i++) {
        simulation_game=create_new_state_of(game, moves[i], false);
        scores[i]=minimax(simulation_game, player, depth+1, NULL);
	delete2DArray(simulation_game.box, 3, 3);
    }

    int best_score;
    if (game.current_player==player) {
        int max_score_index;
        max_score_index=get_max_element_of_array(scores, total_moves); // total_moves=total scores
	best_score=scores[max_score_index];
	if (depth==1) {
	    move[0]=moves[max_score_index][0];
	    move[1]=moves[max_score_index][1];
	}
    } else {
        int min_score_index;
        min_score_index=get_min_element_of_array(scores, total_moves);
	best_score=scores[min_score_index];
    }
    free(scores);
    delete2DArray(moves, total_moves, 2);
    return best_score;
}

st_game create_new_state_of(st_game game, int move[2], bool removeBox)
{
    st_game lc_game = game;
    lc_game.box=createCopy2DArray(game.box, 3, 3); // because box is pointer
    if (removeBox==true)
	delete2DArray(game.box, 3, 3);

    int x=move[0], y=move[1];
    if (lc_game.box[x][y]==-1) {
        lc_game.box[x][y]=lc_game.current_player;
        lc_game.current_player=1-lc_game.current_player; // change player turn
        return lc_game;
    }
    gotoxy(10, 19); printf("Warning: the move is not valid.");
    exit(0);
}

int score(st_game game, int player, int depth)
{
    game.current_player=1-game.current_player;
    int opponent=1-player;
    if (game.current_player==player)
        return 10-depth;
    else if (game.current_player==opponent)
        return -10+depth;
}

bool isOver(int **game_box)
{
    int x, y, **box;
    bool over=true;
    box=createCopy2DArray(game_box, 3, 3);
    // vertical check
    for (x=0; x<3; x++) {
        for (y=1; y<3; y++) {
            if (box[x][y]!=-1 && box[x][y]!=3) {
		if (box[x][y]!=box[x][y-1]) over=false;
		if (box[x][y]==-1) over=false;
		if (over==false) break;
	    } else {
		over=false;
		break;
	    }
	} 
	if (over==true) return true;
	over=true;
    }
    // horizontal check
    for (y=0; y<3; y++) {
        for (x=1; x<3; x++) {
            if (box[x][y]!=-1 && box[x][y]!=3) {
                if (box[x][y]!=box[x-1][y]) over=false;
                if (box[x][y]==-1) over=false;
		if (over==false) break;
            } else {
		over=false;
		break;
	    }
        }
	if (over==true) return true;
	over=true;
    }
    // diagonal 1 check
    for (x=1; x<3; x++) {
	y=x;
        if (box[x][y]!=-1 && box[x][y]!=3) {
	    if (box[x][y]!=box[x-1][y-1]) over=false;
	    if (box[x][y]==-1) over=false;
	    if (over==false) break;
	} else {
	    over=false;
	    break;
	}
    }
    if (over==true) return true;
    over=true;
    // diagonal 2 check
    for (x=1; x<3; x++) {
	y=2-x;
        if (box[x][y]!=-1 && box[x][y]!=3) {
	    if (box[x][y]!=box[x-1][y+1]) over=false;
	    if (box[x][y]==-1) over=false;
	    if (over==false) break;
	} else {
	    over=false;
	    break;
	}
    }
    return over;
}

bool isDraw(int **box)
{
    int x, y;
    for (x=0; x<3; x++) {
	for (y=0; y<3; y++) {
	    if (box[x][y]==-1) return false;
	}
    }
    return true;
}

int count_available_moves(int **box)
{
    int x, y, count=0;
    for (x=0; x<3; x++) {
        for (y=0; y<3; y++) {
            if (box[x][y]==-1) count++;
        }
    }
    return count;
}

int** get_available_moves(int len, int **box)
{
    int x, y, ptr=-1;
    int **moves=create2DArray(len, 2);
    for (x=0; x<3; x++) {
        for (y=0; y<3; y++) {
            if (box[x][y]==-1) {
                ptr++;
                moves[ptr][0]=x;
                moves[ptr][1]=y;
            }
        }
    }
    return moves;
}

void appendLocalToGlobal(st_game global_game, st_game local_game, int turn)
{
    int x, y, x_pos, y_pos;
    for (x=0; x<3; x++) {
        for (y=0; y<3; y++) {
            /* because this board have 9 child board */
            if (turn>=6 && turn<=8) { // child board ke 6 - 8
                y_pos=y+3*(turn-6);
                x_pos=x+6;
            }
            if (turn>=3 && turn<=5) { // child board ke 3 - 5
                y_pos=y+3*(turn-3);
                x_pos=x+3;
            }
            if (turn>=0 && turn<=2) { // child board ke 0 - 2
                y_pos=y+3*turn;
                x_pos=x;
            }
            global_game.map_box[x_pos][y_pos]=local_game.box[x][y];
        }
    }
}

int suit()
{
    // this deciding who are smart player and who are dump player 
    smart_player=rand() % 2;
    dump_player=1-smart_player; // opposite of smart player

    return rand() % 2; // this deciding who are first player
}

st_game initialize_game(st_game game, int player)
{
    game.current_player=player; // set current player
    game.map_box=NULL; // set Null since only global game need it
    game.box=create2DArray(3, 3);
    int x,y;
    for (x=0; x<3; x++) {
        for (y=0; y<3; y++) {
            game.box[x][y]=-1; // fill all box with -1
        }
    }
    return game;
}

st_game initialize_global_game(st_game game, int player)
{
    int x,y;
    game.current_player=player;
    game.box=create2DArray(3, 3);
    for (x=0; x<3; x++) {
        for (y=0; y<3; y++) {
            game.box[x][y]=-1;
        }
    }
    game.map_box=create2DArray(9, 9);
    for (x=0; x<9; x++) {
        for (y=0; y<9; y++) {
            game.map_box[x][y]=-1; // fill all box with -1
        }
    }
    return game;
}

void createBoard()
{
    //system("cls");
    gotoxy(1,1 ); printf("    1   2   3   4   5   6   7   8   9  ");
    gotoxy(1,2 ); printf("  -------------------------------------");
    gotoxy(1,3 ); printf("1 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,4 ); printf("  ------------*-----------*------------");
    gotoxy(1,5 ); printf("2 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,6 ); printf("  ------------*-----------*------------");
    gotoxy(1,7 ); printf("3 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,8 ); printf("  *************************************");
    gotoxy(1,9 ); printf("4 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,10); printf("  ------------*-----------*------------");
    gotoxy(1,11); printf("5 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,12); printf("  ------------*-----------*------------");
    gotoxy(1,13); printf("6 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,14); printf("  *************************************");
    gotoxy(1,15); printf("7 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,16); printf("  ------------*-----------*------------");
    gotoxy(1,17); printf("8 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,18); printf("  ------------*-----------*------------");
    gotoxy(1,19); printf("9 !   !   !   *   !   !   *   !   !   !");
    gotoxy(1,20); printf("  -------------------------------------");
    gotoxy(1,21); printf("\n");
}

void updateBoard(int **box, int turn)
{
    char simbol;
    int x, y, x_pos, y_pos;
    createBoard();
    for (x=0; x<9; x++) {
        for (y=0; y<9; y++) {
            simbol='\0';
            if (box[x][y] != -1)
                simbol=box[x][y]==player1?player1_symbol:player2_symbol;
            y_pos=y;
            x_pos=x;
            // dont confuse, Y here is column and X here is row
            gotoxy(5+(y_pos*4), 3+(x_pos*2)); putchar(simbol);
        }
    }
    gotoxy(1,23); putchar('\0'); // this is important
}

int** create2DArray(int height, int width)
{
    int i;
    int **x=(int **) malloc(height * sizeof(int*));
    for (i=0; i<height; i++) {
        x[i]=(int *) malloc(width * sizeof(int));
    }
    return x;
}

void delete2DArray(int **arr, int height, int width)
{
    int i;
    for (i=0; i<height; i++) {
        free(arr[i]);
    }
    free(arr);
}

int** createCopy2DArray(int **arr_ori, int height, int width)
{
    int i;
    int **arr_target=create2DArray(height, width);
    for (i=0; i<height; i++) {
        memcpy(arr_target[i], arr_ori[i], height * sizeof(int));
    }
    return arr_target;
}

int get_max_element_of_array(int arr[], int n)
{
    int i, max;
    max=0;
    for (i=1; i<n; i++) {
        if (arr[i]>arr[max]) {
            max=i;
        }
        // randomly select max if there is same high score
        if (arr[i]==arr[max]) {
            if (rand() % 2==1) max=i;
        }
    }
    return max;
}

int get_min_element_of_array(int arr[], int n)
{
    int i, min;
    min=0;
    for (i=1; i<n; i++) {
        if (arr[i]<arr[min]) {
            min=i;
        }
        // randomly select min if there is same high score
        if (arr[i]==arr[min]) {
            if (rand() % 2==1) min=i;
        }
    }
    return min;
}

// defines gotoxy() for ANSI C compilers.
bool gotoxy(int x, int y) {
    if ((x == 0) || (y == 0))
	return false;
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    return true;
}

void setColor(int color)
{
    HANDLE hStdout;
 
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, color);
}
