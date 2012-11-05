// Peter Inslee, CS410 Mastery in Programming, Summer 2011
// game_of_life.c: Demonstrates John Conway's "Game of Life" using ASCII text. Reads a plain text file as
//                 input, and using the rules of the game, computes the next "generation" of cells (chars)
//                 from the given input. Will continue for a fixed default number of generations, printing
//                 each to a file called "output.txt" in the current working directory. (The file will be
//                 overwritten if present already before execution, or created if not present.) The user
//                 may direct the program to load data from a specific file provided as an optional command
//                 line argument. The program looks for and attempts to open (for reading) a text file
//                 "input.txt" by default, if no alternative is provided. The program quits with an error
//                 message printed to stderr if it encounters problems reading or writing either of these.
#include <errno.h>      // to use 'errno' and 'perror()';
#include <stdio.h>      // to use 'fgetc()', 'fputc()', 'printf()', 'fprintf()', 'fopen()' and 'fclose()';
#include <stdlib.h>     // to use 'exit()';
#define ROWS 48         // hardwired height of the grid, that is, the number of rows in each buffer;
#define COLUMNS 64      // hardwired width of the grid, that is, the number of columns in each buffer;
#define GENERATIONS 30  // number of iterations for which to run the cellular automaton;

extern int errno;

typedef enum { NOT_FINISHED, FINISHED_ROW, FINISHED_GRID } state;
typedef enum { ONE_CELL, ONE_ROW } mode;

typedef struct cell_pointer_struct {    // used for traversing the first buffer, to load it with input;
    int    row;
    int    column;
    char * cell;
} cell_pointer;

// given a cell within the current grid (at 'row_position, column_position'), this function determines its
// neighbors, and counts how many of them are living; it handles special cases of cells at the border by
// treating the grid as a thorus, essentially wrapping the grid around in both dimensions; a cell in the
// first column, for example, has 3 neighbors "to the left" (in the "column_before"), which are actually
// in the last column to the right; likewise, a cell at the bottom has 3 neighbors "below it" (in the
// "row_below"), which are actually in the first row at the top; (note that corner cells have such
// neighbors in both dimensions); once this detail has been handled, then this function counts how many of
// the 8 neighbors of the given cell are living, and returns this integer to the caller;
int count_living_neighbors( char current_grid[][COLUMNS], int row_position, int column_position )
{
    int row_above;
    int row_below;
    int column_before;
    int column_after;
    int living_neighbors = 0;

    // handle boundary cases
    // cell's row less 1 indicates the row above it
    if ( -1 == ( row_above = ( row_position - 1 ) ) ) {
        // if no row above cell, make the last (bottom) row its neighbor
        row_above = ( ROWS - 1 );
    }
    // cell's column less 1 indicates the column before it (on its left)
    if ( -1 == ( column_before = ( column_position - 1 ) ) ) {
        // if no column before cell, make the last (rightmost) column its neighbor
        column_before = ( COLUMNS - 1 );
    }
    // cell's row plus 1 indicates the row below it
    if ( ROWS == ( row_below = ( row_position + 1 ) ) ) {
        // if no row below cell, make the first (top) row its neighbor
        row_below = 0;
    }
    // cell's column plus 1 indicates the column after it (on its right)
    if ( COLUMNS == ( column_after = ( column_position + 1 ) ) ) {
        // if no column after cell, make the first (leftmost) column its neighbor
        column_after = 0;
    }

    // count the living neighbors of the given cell;
    // note that cells contain either ' ' or 'o';
    if ( 'o' == current_grid[row_above][column_before] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_above][column_position] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_above][column_after] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_position][column_before] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_position][column_after] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_below][column_before] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_below][column_position] ) {
        living_neighbors++;
    }
    if ( 'o' == current_grid[row_below][column_after] ) {
        living_neighbors++;
    }

    return living_neighbors;
}

// implements John Conway's rules for the 'Game of Life' automaton; for each cell in the given grid, count
// its neighbors and determine if it spawns to life, stays dead, survives, or dies, and mark this result in
// the next grid (the next generation); a cell (living or dead) having exactly 3 living neighbors
// is guaranteed to be alive in the next generation, a living cell with with 2 (or 3) neighbors will stay
// living in the next generation; if neither of these conditions is met, the cell will die or stay dead;
// note that cells at the border are considered to have neighbors at the opposite border, as though the
// grid "wraps around" from top to bottom and side to side, modeling a thorus; (this detail is handled in
// the function 'count_living_neighbors()');
void generate_grid( char current_grid[][COLUMNS], char next_grid[][COLUMNS] )
{
    int r, c;

    for ( r = 0; r < ROWS; r++ ) {
        for ( c = 0; c < COLUMNS; c++ ) {
            // initialize corresponding cell in next grid before assigning it a value
            next_grid[r][c] = ' ';
            // if ( 3 neighbors are alive || ( this cell is living && 2 neighbors are living ) ) {
            if ( 3 == count_living_neighbors( current_grid, r, c )
            || ( 'o' == current_grid[r][c] && 2 == count_living_neighbors( current_grid, r, c ) ) ) {
                next_grid[r][c] = 'o';  // mark this cell as living
            }                           // else do nothing, the cell stays dead or dies
        }
    }
}

// this function prints out the results stored in a given buffer (grid); it prints the generation number,
// and then all the cells of a given iteration of the automaton (i.e., one generation), and surrounds this
// grid with a frame to dilineate its boundaries;
void print_grid( char current_grid[][COLUMNS], int generation, FILE * output_file_pointer )
{
    int r, c;

    // print top of frame
    fprintf( output_file_pointer, "Generation: %d\n+", generation );
    for ( c = 0; c < COLUMNS; c++ ) {
        fputc( '-', output_file_pointer );
    }
    fprintf( output_file_pointer, "+\n" );
    // print body of grid
    for ( r = 0; r < ROWS; r++ ) {
        fputc( '|', output_file_pointer );      // print left side of frame
        for ( c = 0; c < COLUMNS; c++ ) {
            fputc( current_grid[r][c], output_file_pointer );
        }
        fprintf( output_file_pointer, "|\n" );  // print right side of frame
    }
    // print bottom of frame
    fputc( '+', output_file_pointer );
    for ( c = 0; c < COLUMNS; c++ ) {
        fputc( '-', output_file_pointer );
    }
    fprintf( output_file_pointer, "+\n" );
}

// this is a facilitator function called by 'load_input()' to handle calculating where in the buffer to
// store input as it is scanned from the input file; it advances the target either one cell in the current
// row, and/or it advances the target to the beginning of the next row, and returns the resulting state
state advance_target( cell_pointer * target, mode advance_mode, char starting_grid[][COLUMNS] )
{
    state target_state = NOT_FINISHED;

    if ( ONE_CELL == advance_mode ) {               // caller requests that we advance target by one cell;
        if ( ( target->column + 1 ) < COLUMNS ) {   // if doing so keeps us in this row, then do it,
            target->cell = &( starting_grid[target->row][++target->column] );   // and return NOT_FINISHED;
        } else {
            target_state = FINISHED_ROW;            // otherwise we've finished loading this row;
        }
    } // either we just finished loading a row, or the caller requests we advance the target by a row;
    if ( FINISHED_ROW == target_state || ONE_ROW == advance_mode ) {
        if ( ( target->row + 1 ) < ROWS ) {         // if doing so keeps us in the grid, then do it;
            target->column = 0;                     // reset target to beginning of next row,
            target->cell = &( starting_grid[++target->row][target->column] );   // and either return
                                                    // FINISHED_ROW if request was to advance one cell, or
                                                    // NOT_FINISHED if request was to advance one row;
        } else {
            target_state = FINISHED_GRID;           // otherwise we've finished loading this grid
        }
    }

    return target_state;
}

// given a filename and a buffer (grid), this function attempts to open the corresponding file for reading
// and, if successful, loads the buffer with the character data found in the file; if the buffer is not
// large enough to accommodate the input (in either or both dimensions), the extra input is ignored, in
// essence "cropping" the given input to fit the given buffer size; on the other hand, if the buffer is
// larger than necessary, then the unused portion of the buffer is simply initialized with empty cells to
// make up the difference; once the buffer is loaded, the input file is closed; returns 1 if successful,
// or 0 if errors are encountered;
int load_input( char * file_name, char starting_grid[][COLUMNS] )
{
    FILE * file_pointer;
    if ( NULL == ( file_pointer = fopen( file_name, "r" ) ) ) {
        fprintf( stderr, "error: in 'load_input()', call to 'fopen(%s, 'r')' failed\n", file_name );
        perror( "     " );
        return 0;
    }

    int c = ' ';                        // used to store characters scanned from input file one at a time
    cell_pointer target;                // used as a "write head" to load values into the buffer
    state target_state = NOT_FINISHED;  // refers to target's location in buffer, starts off ready to load

    target.row = 0;                     // initialize target to point to first cell in buffer to be loaded
    target.column = 0;
    target.cell = &( starting_grid[0][0] );

    // start scanning input file and loading character data into the starting buffer (grid);
    // silently handle any size mismatches between input file and the buffer by cropping or expanding;
    while ( FINISHED_GRID != target_state   // keep scanning input file until buffer's full or End Of File;
            &&        EOF != c              // required to catch EOF after the 'do {} while ()' scan below
            &&        EOF != ( c = fgetc( file_pointer ) ) ) { // to avoid a re-scan past the EOF here
        switch ( ( char )c ) {
            case '\n':  target_state = advance_target( &target, ONE_ROW, starting_grid );
                        break;              // line in file ended, so leave the rest of row empty in grid
            case 'o':   *target.cell = 'o'; // load character value into buffer marking a live cell
                        target_state = advance_target( &target, ONE_CELL, starting_grid );
                        break;              // silently treat any other character like a ' ', leave dead
            default:    target_state = advance_target( &target, ONE_CELL, starting_grid );
                        break;              // silently treat any other character like a ' ', leave dead
        }
        if ( FINISHED_ROW == target_state ) { // we filled buffer row, but not done scanning line in file;
            // that is, the input in this line of the file is "wider" than the width of our grid, so we
            // scan the file until finding the next newline or end of file, ignoring everything until then;
            do {
                c = fgetc( file_pointer );
            } while ( EOF != c && '\n' != ( char )c );  // EOF is caught again in while loop above; but,
        }                                               // '\n' results in call to 'fgetc()' again above
    }

    if ( EOF == fclose( file_pointer ) ) {
        fprintf( stderr, "error: in 'load_input()', call to 'fclose(%s)' failed\n", file_name );
        perror( "     " );
        return 0;
    }

    return 1;   // return 1 on success, 0 on failure
}

// main entry point for the program; initializes parameters, reads input file name argument if provided,
// declares two 2-dimensional arrays used as buffers to simulate the evolution of the cellular automaton
// grid, loads initial generation from input file into grid, runs automaton for 30 generations, prints
// results of evolution silently to file "output.txt" using a grid of 48 rows by 64 columns, (not including
// the additional frame which is printed at the perimeter of the grid), and prints error messages and exits
// if any part of this process fails;
int main( int argc, char ** argv )
{
    char even_grid[ROWS][COLUMNS];  // for double-buffering the grid, used for even-numbered generations
    char odd_grid[ROWS][COLUMNS];   // other buffer, used for odd-numbered generations
    int g, r, c;                    // indices
    char * input_filename;                 // "input.txt" if user does not provide a different file
    char * output_filename = "output.txt"; // hardwired to always create or overwrite same output file
    FILE * output_file_pointer;            // used to print results to file as they are generated

    for ( r = 0; r < ROWS; r++ ) {         // initialize buffers so they are full of dead cells (spaces)
        for ( c = 0; c < COLUMNS; c++ ) {
            even_grid[r][c] = ' ';
            odd_grid[r][c] = ' ';
        }
    }

    // allow user to specify input file as command line argument to program
    if ( 1 < argc ) {   // if arguments are given, assume first is the name of input file
        input_filename = argv[1];
    } else {            // no input filename provided, so use default name, "input.txt"
        input_filename = "input.txt";
    }

    if ( !load_input( input_filename, even_grid ) ) {
        fprintf( stderr, "usage: %s [ input_file ]\n", argv[0] );
        fprintf( stderr, "       quitting...\n" );
        exit( EXIT_FAILURE );
    }

    if ( NULL == ( output_file_pointer = fopen( output_filename, "w" ) ) ) {
        fprintf( stderr, "error: in 'main()', call to 'fopen(%s, 'w')' failed\n", output_filename );
        perror( "     " );
        fprintf( stderr, "usage: %s [ input_file ]\n", argv[0] );
        fprintf( stderr, "       quitting...\n" );
        exit( EXIT_FAILURE );
    }

    // run the 'Game of Life' on the provided input;
    for ( g = 0; g < GENERATIONS; g++ ) {   // iterate automaton for hardwired number of times;
        if ( 0 == g % 2 ) {                 // modulo for double-buffering; even generations use even_grid;
            print_grid( even_grid, g, output_file_pointer );   // once it's loaded or computed, print it;
            generate_grid( even_grid, odd_grid );             // use even_grid to compute new odd_grid;
        } else {                            // odd generations use odd grid
            print_grid( odd_grid, g, output_file_pointer );    // once it's computed, print it;
            generate_grid( odd_grid, even_grid );             // use odd_grid to compute new even_grid;
        }
    }

    if ( EOF == fclose( output_file_pointer ) ) {
        fprintf( stderr, "error: in 'main()', call to 'fclose(%s)' failed\n", output_filename );
        perror( "     " );
        fprintf( stderr, "usage: %s [ input_file ]\n", argv[0] );
        fprintf( stderr, "       quitting...\n" );
        exit( EXIT_FAILURE );
    }

    exit( EXIT_SUCCESS );
}

