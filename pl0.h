#include <stdio.h>
#include <malloc.h>																				// added by nanahka 17-11-26

#define TRUE	   1
#define FALSE	   0

#define NRW        14     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       13     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers
#define MAXARYDIM  10	  // maximum number of dimensions of an array							// added by nanahka 17-11-12
#define MAXARYVOL  200	  // maximum volume of a dimension of an array							// added by nanahka 17-11-12
#define MAXFUNPMT  10	  // maximum number of parameters of a procedure						// added by nanahka 17-11-20

#define MAXADDRESS 32767  // maximum numerical constant											// modified by nanahka 17-11-20
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array
//#define INITLIST   10	  // initial size of true/false lists									// added by nanahka 17-11-26
//#define INCRELIST  5	  // increment of size of true/false lists

#define MAXSYM     39    // maximum number of symbols

#define STACKSIZE  1000   // maximum storage

#define CONST_EXPR 0	  // the expression is constant											// added by nanahka 17-11-14
#define UNCONST_EXPR 1	  // the expression is not constant

#define TABLE_BEGIN 0	  // the beginning index of the TABLE, used in position()				// added by nanahka 17-11-14

#define MAX_EXIT 20      //the max number of exit  ADDED BY LZP
#define MAX_RET  20      //max number of return in evert procedure

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_LSQUARE,				// 2 square brackets added by nanahka 17-11-12
	SYM_RSQUARE,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	//SYM_CALL,					// deleted by nanahka 17-11-20
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_NOT,					// added 17-11-26
	SYM_OR,
	SYM_AND,
	SYM_AMPERSAND,				// added by nanahka 17-11-20
	SYM_ELSE,
	SYM_FOR,                     //added by lzp
	SYM_RETURN,
	SYM_EXIT
};	// total number = MACRO MAXSYM, maintenance needed!!!

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, LODI, STO, STOI, CAL, INT, JMP, JPC, JND, JNDN								// added by nanahka 17-11-14
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_AND, OPR_OR															// added by nanahka 17-11-26
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",									// unused
/*  7 */    "Declaration/Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "",																				// change to calling operator error
/* 16 */    "'then' expected.",
/* 17 */    "'end' expected.",					// modified by nanahka 17-11-13
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relop not admitted in const expression.", // modified by nanahka 17-11-26
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by an expression.",	// modified by nanahka 17-11-26
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "Missing ';'.",						// 26 added by nanahka 17-11-13
/* 27 */    "Applying the index operator on non-array.",		// 27-29 added by nanahka 17-11-14
/* 28 */    "Variables can not be in a const expression.",
/* 29 */    "Too few subscripts.",
/* 30 */    "Too many subscripts.",				// 30 added by nanahka 17-11-15
/* 31 */    "Redeclaration of an identifier.",	// 31 added by nanahka 17-11-20
/* 32 */    "There are too many levels.",
/* 33 */	"Numeric constant expected.",		// 33-36 added by nanahka 17-11-12
/* 34 */	"']' expected.",
/* 35 */	"There are too many dimensions.",
/* 36 */	"Volume of a dimension is out of range.",
/* 37 */	"'=' expected.",					// 37 added by nanahka 17-11-13
/* 38 */	"'[' expected.",					// 38 added by nanahka 17-11-14
/* 39 */	"Too many parameters in a procedure.",	// 39-42 added by nanahka 17-11-20
/* 40 */	"Missing ',' or ')'.",
/* 41 */	"Array type as parameter forbidden.",
/* 42 */	"Too few parameters in a procedure.",
/* 43 */      "'(' expected.",
/* 44 */      "there must be a variable in 'for' statement.",
/* 45 */       "you must return a constant.",
/* 46 */       "no more exit can be added.",
/* 47 */       "'else' expected.",
/* 48 */       "another '|' is expected."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  *ptr;		 // a link list containing dimensions of an array								// added by nanahka 17-11-12
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
int  tx_b = 0;	 // index of the beginning of current block in TABLE
int  *list[2] = {}; // list[0]: f_list, list[1]: t_list
//int  *t_list;	 // array of code indices of JPCs to the TRUE address of a logical expr
//int  *f_list;	 // array of code indices of JPCs to the FALSE address of a logical expr
int  cx_exit[MAX_EXIT];     //to mark the code of 'exit'     ADDED BY LZP
int  i_exit=0;    //to count the number of exit
int  cx_ret[MAX_RET];      //to mark the code of 'return'
int  i_ret=0;        //to count the number of 'return'

char line[80];

instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", /*"call",*/ "const", "do", "end","if",												// deleted by nanahka 17-11-20
	"odd", "procedure", "then", "var", "while",
	"else","for","return" ,"exit"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, /*SYM_CALL,*/ SYM_CONST, SYM_DO, SYM_END,								// deleted by nanahka 17-11-20
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_ELSE, SYM_FOR, SYM_RETURN, SYM_EXIT
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LSQUARE, SYM_RSQUARE, SYM_NOT																	// added by nanahka 17-11-26
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '[', ']', '!'						// added by nanahka 17-11-26
};

#define MAXINS   12																				// modified by nanahka 17-11-26
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "LODI", "STO", "STOI", "CAL", "INT", "JMP", "JPC", "JND", "JNDN"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int  *ptr;							// added by nanahka 17-11-12 for dimensions in an array
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
	int   *ptr;							// added by nanahka 17-11-12
} mask;

//typedef struct																					// added by nanahka 17-11-21
//{
//	short level;
//	short address;
//} mask_val;

FILE* infile;

// EOF PL0.h
