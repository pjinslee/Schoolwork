///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                       //
// octagons.c:      (Submitted by Peter Inslee to Dr. Bryant York on Monday, 27 February 2012 in partial //
// fulfillment of the requirements of CS350, "Algorithms and Complexity" at the PSU Maseeh College of    //
// Engineering and Computer Science, Portland, OR.) This is a near solution to the Waterloo Programming  //
// Contest problem "Problem B: Octagons", posted at "http://acm.student.cs.uwaterloo.ca/~acm00/111002/". //
//                                                                                                       //
// PROBLEM DESCRIPTION                                                                                   //
// Given an infinite hyperbolic tessellation of octagons structured as a graph of vertices (of degree    //
// three), there exists an isomorphism of the graph which maps any vertex x onto any other vertex y.     //
// When every edge is given a label from the set {a,b,c} in such a way that every vertex has all three   //
// types of edges incident on it, the labels alternate around each octagon. A path in this graph         //
// (starting from any vertex) can be specified by a sequence of edge labels. This program, when given a  //
// sequence of labels such as "abcbcbcabcaccabb", returns "CLOSED" if the path ends on the same vertex   //
// where it starts, and returns "OPEN" otherwise.                                                        //
//                                                                                                       //
// INPUT SPECIFICATION                                                                                   //
// This version differs from the problem solution as specified in that it accepts its input from the     //
// command-line instead of from a file. It prompts the user for a sequence of at most 40 'a's 'b's and   //
// 'c's. It silently ignores bad characters in the string, and continues parsing the input until either  //
// the line ends or 40 acceptable edge labels have been read. Any additional input buffered in 'stdin'   //
// is ignored.                                                                                           //
//                                                                                                       //
// SAMPLE PROMPT, INPUT AND OUTPUT                                                                       //
//                                                                                                       //
// Please enter a string of up to 40 'a's, 'b's and 'c's, (or 'q' to quit):                              //
// > ababacbcbcababacbcbcababacbcbcababacbcbc                                                            //
//                                                                                                       //
//     The given input path 'ababacbcbcababacbcbcababacbcbcababacbcbc' is                                //
//     CLOSED, (i.e., it represents a cycle).                                                            //
//                                                                                                       //
// Note that the restriction on the size of the input string (to 40 characters) is fundamental to the    //
// challenge of the problem: a single octagon surrounded by 8 others has a bounding outer perimeter of   //
// 40 edges, so the problem designers wanted to be sure we could recognize such a path as a cycle.       //
//                                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#define MAX 40

typedef enum {FALSE, TRUE} Boolean;
typedef struct Node_t* NodePtr;
typedef struct Node_t{
    NodePtr a;
    NodePtr b;
    NodePtr c;
    Boolean start;
    Boolean visited;
    NodePtr next_to_free;
} Node;

NodePtr free_list = 0; // global for garbage collection

void free_nodes(NodePtr n)
{
    NodePtr next;
    while (!n->start) { // free all but start node
        next = n->next_to_free;
        free(n);
        n = next;
    }
    free_list = n;
}

void connect(NodePtr n1, NodePtr n2, char edge)
{
    switch (edge) {
        case 'a':   n1->a = n2;
                    n2->a = n1;
                    break;
        case 'b':   n1->b = n2;
                    n2->b = n1;
                    break;
        case 'c':   n1->c = n2;
                    n2->c = n1;
                    break;
    } 
}

void initialize_node(NodePtr n, Boolean start)
{
    n->a = 0;
    n->b = 0;
    n->c = 0;
    n->start = start;
    n->visited = start;
    n->next_to_free = free_list;
    free_list = n;
}

NodePtr make_node(NodePtr n, char edge)
{
    NodePtr new_node = (NodePtr)malloc(sizeof(Node));

    initialize_node(new_node, FALSE);
    connect(new_node, n, edge);
    return new_node;
}

NodePtr get_next_node(NodePtr n, char edge)
{
    switch (edge) {
        case 'a':   return n->a;
        case 'b':   return n->b;
        case 'c':   return n->c;
    }
    return 0;
}

int make_octagon(NodePtr first_vertex, char label1, char label2)
{
    NodePtr n = first_vertex;
    NodePtr next = get_next_node(first_vertex, label1);
    int count = 7;
    Boolean flip_to_label1 = FALSE;
    NodePtr loose_end;

    while (next) { // walk around one side of octagon until we either hit a dead end
        if (count < 0) { // or circle all the way around, in which case there's no need to make anything
            return 0;
        }
        count--; // count how many nodes we pass along the way
        n = next;
        if (flip_to_label1) {
            next = get_next_node(n, label1);
            flip_to_label1 = FALSE;
        } else {
            next = get_next_node(n, label2);
            flip_to_label1 = TRUE;
        }
    }
    loose_end = n; // provide a handle to the loose end for connecting to later
    n = first_vertex;
    next = get_next_node(first_vertex, label2);
    flip_to_label1 = TRUE;
    while (next) { // walk around other side until we hit a dead end
        count--; // count how many nodes we pass along the way
        n = next;
        if (flip_to_label1) {
            next = get_next_node(n, label1);
            flip_to_label1 = FALSE;
        } else {
            next = get_next_node(n, label2);
            flip_to_label1 = TRUE;
        }
    } // found other dead end, so now we need to fill in the gap with new nodes and edges
    while (count) {
        count--;
        if (flip_to_label1) {
            next = make_node(n, label2);
            n = next;
            next = get_next_node(n, label1);
            flip_to_label1 = FALSE;
        } else {
            next = make_node(n, label1);
            n = next;
            next = get_next_node(n, label2);
            flip_to_label1 = TRUE;
        }
    } // connect the two loose ends
    if (flip_to_label1) {
        connect(n, loose_end, label2);
    } else {
        connect(n, loose_end, label1);
    }
    return 0;
}

Boolean visit(NodePtr n, char e)
{
    Boolean cycle = FALSE;

    if (n->visited) { // if this node is visited, then no need to make any octagons
        if (n->start) cycle = TRUE; // if this is the start node, then we have found a cycle
    } else { // this node hasn't been visited, so need to try to make a pair of octagons
        n->visited = TRUE; // mark this node as "visited"
        switch (e) {
            case 'a':   if (!n->b->visited) make_octagon(n->b, 'a', 'c');
                        if (!n->c->visited) make_octagon(n->c, 'a', 'b');
                        break;
            case 'b':   if (!n->a->visited) make_octagon(n->a, 'b', 'c');
                        if (!n->c->visited) make_octagon(n->c, 'a', 'b');
                        break;
            case 'c':   if (!n->a->visited) make_octagon(n->a, 'b', 'c');
                        if (!n->b->visited) make_octagon(n->b, 'a', 'c');
                        break;
        }
    }
    return cycle;
}

Boolean is_cycle(char A[], NodePtr start)
{
    int i;
    NodePtr n = start;
    NodePtr next;
    Boolean cycle;

    printf("\n    The given input path '");
    for (i = 0; i < MAX && A[i] != '\n'; i++) { // get next edge from input string
        if (A[i] != 'a' && A[i] != 'b' && A[i] != 'c') {
            continue; // silently ignore bad characters in input
        } else {
            putc(A[i], stdout);
            next = get_next_node(n, A[i]);  // walk edge from this node to next
            cycle = visit(next, A[i]);
            n = next;
        }
    }
    printf("' is\n");
    free_nodes(free_list); // if no more edges, free memory and return "cycle"
    return cycle;
}

void make_first_6_octagons(NodePtr start_node)
{
    make_octagon(start_node, 'a', 'b');
    make_octagon(start_node, 'a', 'c');
    make_octagon(start_node, 'b', 'c');
    make_octagon(start_node->a, 'b', 'c');
    make_octagon(start_node->b, 'a', 'c');
    make_octagon(start_node->c, 'a', 'b');
}

int main()
{
    char A[MAX];
    Node start_node;

    while (1) {
        printf("Please enter a string of up to %d 'a's, 'b's and 'c's, (or 'q' to quit):\n", MAX);
        printf("> ");
        fflush(stdout);
        fgets(A, MAX + 1, stdin);
        if (A[0] == 'q' || A[0] == 'Q') {
            printf("\n    Quitting...\n\n");
            break;
        }
        initialize_node(&start_node, TRUE);
        make_first_6_octagons(&start_node);
        if (is_cycle(A, &start_node)) {
            printf("    CLOSED, (i.e., it represents a cycle).\n\n");
        } else {
            printf("    OPEN, (i.e., it does not represent a cycle.\n\n");
        }
    }
    return 0;
}

