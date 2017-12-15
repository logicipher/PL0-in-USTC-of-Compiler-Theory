#include <stdio.h>

#define TRUE	   1
#define FALSE	   0

#define NRW        18     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       12     // maximum number of symbols in array ssym and csym                    //modified by lzp 27/12/14
#define MAXIDLEN   10     // length of identifiers
#define MAXARYDIM  10	  // maximum number of dimensions of an array							// added by nanahka 17-11-12
#define MAXARYVOL  200	  // maximum volume of a dimension of an array							// added by nanahka 17-11-12
#define MAXFUNPMT  10	  // maximum number of parameters of a procedure						// added by nanahka 17-11-20

#define MAXADDRESS 32767  // maximum numerical constant											// modified by nanahka 17-11-20
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     44    // maximum number of symbols                                             //modified by lzp 17/12/14

#define STACKSIZE  1000   // maximum storage

#define CONST_EXPR 0	  // the expression is constant											// added by nanahka 17-11-14
#define UNCONST_EXPR 1	  // the expression is not constant

#define TABLE_BEGIN 0	  // the beginning index of the TABLE, used in position()				// added by nanahka 17-11-14
#define MAX_CASE 20       //maxinum cases in switsh                              added by lzp 17/12/14
#define INCREMENT 5       //increment preparing for more space if necessary

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
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_NOT,
	SYM_OR,
	SYM_AND,
	SYM_AMPERSAND,				// added by nanahka 17-11-20
	SYM_ELSE,
	SYM_FOR,                     //added by lzp
	SYM_RETURN,            
	SYM_EXIT,
	SYM_SWITCH,
	SYM_CASE,
	SYM_BREAK,
	SYM_DEFAULT,
	SYM_COLON
};	// total number = MACRO MAXSYM, maintenance needed!!!

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, LODI, STO, STOI, CAL, INT, JMP, JPC, EXT											// added by nanahka 17-11-14
};                                                                       //added by lzp 

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_RTN                                //added by lzp 2017/12/10
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
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
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
/* 48 */       "another '|' is expected.",
/* 49 */     "'while' expected .",
/* 50 */    "there must be 'begin' in switch statement.",
/* 51 */    "'case','end',or 'default' is expected .",
/* 52 */    "':' expected ."
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
int true_out[4][10]={0};//短路计算，真值链 
int false_out[4][10]={0};//短路计算，假值链 
int true_count[4]={0};
int false_count[4]={0};
int condition_level=0;
int tx_c=0;         //index of case table,pointing to the current case              //   added by lzp 17/12/14
int maxcase = MAX_CASE;
/*
int cx_exit[MAX_EXIT];     //to mark the code of 'exit'     ADDED BY LZP
int i_exit=0;    //to count the number of exit
int cx_ret[MAX_RET];      //to mark the code of 'return'
int i_ret=0;        //to count the number of 'return'
*/

char line[80];

instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", /*"call",*/ "const", "do", "end","if",												// deleted by nanahka 17-11-20
	"odd", "procedure", "then", "var", "while",
	"else","for","return" ,"exit", "switch", "case" ,"default"                          //add by lzp
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, /*SYM_CALL,*/ SYM_CONST, SYM_DO, SYM_END,								// deleted by nanahka 17-11-20
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_ELSE,SYM_FOR,SYM_RETURN,SYM_EXIT , SYM_SWITCH, SYM_CASE,
	SYM_BREAK, SYM_DEFAULT                                                                                    //added by lzp 2017/12/12
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_AMPERSAND, SYM_COLON																			// added 17-11-20
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '&',':'								// modified by lzp 2017/12/12
};

#define MAXINS   11 																	// modified by lzp 17/12/14
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "LODI", "STO", "STOI", "CAL", "INT", "JMP", "JPC", "EXT"        //added by lzp 2017/12/12
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

typedef struct                                //added by lzp 17/12/14
{
	int t;                   //the condition of switch
	int c;                   //the index of first ins of every case
	int flag;                //to mark break
	int cx_bre               //break cx
}casetab;
casetab *switchtab;

FILE* infile;

// EOF PL0.h
