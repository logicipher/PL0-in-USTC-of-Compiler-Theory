#include <stdio.h>
#include <malloc.h>

#define TRUE	   		1																			// added by nanahka 17-11-26
#define FALSE	   		0

#define NRW        		26     // number of reserved words
#define TXMAX      		500    // length of identifier table
#define MAXNUMLEN  		14     // maximum number of digits in numbers
#define NSYM       		14     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   		10     // length of identifiers
#define MAXARYDIM  		10	   // maximum number of dimensions of an array							// added by nanahka 17-11-12
#define MAXARYVOL  		200	   // maximum volume of a dimension of an array							// added by nanahka 17-11-12
#define MAXFUNPMT  		10	   // maximum number of parameters of a procedure						// added by nanahka 17-11-20
#define MAXELIF			20	   // maximum number of elifs											// added by nanahka 17-12-19

#define MAXADDRESS 		32767  // maximum numerical constant										// modified by nanahka 17-11-20
#define MAXLEVEL   		32     // maximum depth of nesting block
#define CXMAX      		500    // size of code array

#define MAXSYM     		52     // maximum number of symbols

#define STACKSIZE  		1000   // maximum storage

#define CONST_EXPR 		0	   // the expression is constant										// added by nanahka 17-11-14
#define UNCONST_EXPR 	1	   // the expression is not constant

#define TABLE_BEGIN 	0	   // the beginning index of the TABLE, used in position()				// added by nanahka 17-11-14
#define MAX_CASE   		20     //maxinum cases in switsh                              				// added by lzp 17/12/14
#define INCREMENT  		5      //increment preparing for more space if necessary
#define MAX_CONTROL 	50     //maxinum control statement


#define PMT_PROC   		1	   // indicate that this name is a procedure and a parameter of a procedure	// added by nanahka 17-12-16
#define NON_PMT_PROC 	0	   // otherwise

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
	SYM_TRUE,					// added by nanahka 17-12-19
	SYM_FALSE,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_NOT,					// added 17-11-26
	SYM_OR,
	SYM_AND,
	SYM_AMPERSAND,				// added by nanahka 17-11-20
	SYM_ELSE,
	SYM_ELIF,					// added by nanahka 17-12-19
	SYM_FOR,                    // added by lzp
	SYM_RETURN,
	SYM_EXIT,
	SYM_SWITCH,					// added by lzp 17/12/15
	SYM_CASE,
	SYM_DEFAULT,
	SYM_BREAK,
	SYM_CONTINUE,
	SYM_COLON,
	SYM_GOTO,
	SYM_VOID,					// added by nanahka 17-12-18
	SYM_INT,

	SYM_PRINT					// added by nanahka 17-12-20
};	// total number = MACRO MAXSYM, maintenance needed!!!

enum idtype																						// merged by nanahka 17-12-15
{
	ID_VOID, ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY, ID_POINTER, ID_LABEL             //added by lzp 17/12/16
};

enum opcode
{
	LIT, OPR, LOD, LODI, LODS, LEA, STO, STOI, STOS,
	CAL, CALS, INT, JMP, JPC, JND, JNDN, RET, EXT, JET, PRNT									// modified by nanahka 17-12-19
};

enum oprcode
{
	/* OPR_RET, */OPR_NEG, OPR_ADD, OPR_MIN,													// deleted by nanahka 17-12-19
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_AND, OPR_OR,                               							// added by nanahka 17-11-26
	OPR_ITOB																					// added by nanahka 17-12-19
};

enum environment                                           										//added by lzp 17/12/16
{
	ENV_NULL, ENV_DO, ENV_WHILE, ENV_FOR, ENV_SWITCH                    //four kind env:do-while,while,for,switch
};

enum control                                                       								//added by lzp 17/12/16
{
	CON_NULL, CON_BREAK, CON_CONTINUE                                   //mark the type of control statement
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
/*  2 */    "There must be a number to follow '='.",						// unused
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
/* 14 */    "There must be an identifier to follow the 'call'.",			// unused
/* 15 */    "There must be an identifier to follow '&'.",		// merged by nanahka 17-12-15
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
/* 41 */	"Non-array type/incorrect indices.",	// modified by nanahka 17-12-16
/* 42 */	"Too few parameters in a procedure.",
/* 43 */    "'(' expected.",
/* 44 */    "There must be a variable in 'for' statement.",
/* 45 */    "you must return a constant.",
/* 46 */    "no more exit can be added.",
/* 47 */    "'else' expected.",								// unused
/* 48 */    "another '|' is expected.",
/* 49 */     "'while' expected .",
/* 50 */    "there must be 'begin' in switch statement.",                 //added by lzp 17/12/14
/* 51 */    "'case','end',or 'default' is expected .",
/* 52 */    "':' expected .",
/* 53 */	"The symbol can not be as the beginning of an lvalue expression.", // 53-56 added by nanahka 17-12-16
/* 54 */	"Incorrect type as an lvalue expression.",
/* 55 */	"The symbol can not be as the beginning of a function call.",
/* 56 */	"Non-procedure type/incorrect parameter types.",
/* 57 */    "procedure can not be in a const factor.",		// unused
/* 58 */    "label must be followed by a statement.",
/* 59 */    "Undeclared label.",
/* 60 */	"Procedure can not be in const expression.",					// added by nanahka 17-12-16
/* 61 */	"'void' or 'int' needed before a procedure declared.",			// 61-64 added by nanahka 17-12-18
/* 62 */	"There must be an identifier to follow 'void' or 'int'.",
/* 63 */	"Void value not ignored as it ought to be.",
/* 64 */	"Applying function calling operator on non-function.",
/* 65 */	"Return a value in function returning void.",					// 65-68 added by nanahka 17-12-19
/* 66 */	"Return void in function returning a value.",
/* 67 */	"Too many elifs.",
/* 68 */	"'elif' or 'else' expected.",
/* 69 */	"Assigning non-array type to an array.",						// 69-70 added by nanahka 17-12-20
/* 70 */	"Assigning non-function type to a function.",
/* 71 */    "id can't be used as a label."									// added by lzp 17-12-19
};

typedef struct type comtyp;
//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
comtyp  *ptr;	 // a dynamic array containing elements composing a composite type				// modified by nanahka 17-12-15
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
int  cc_p;		 // cc of the first ch of current sym											// added by nanahka 17-12-20
int  sym_p;		 // sym before current sym was read

int  env;        // mark the type of environment where break,continue is                       //added by lzp 17/12/16
int  head;
int  tail;       // mark beginning and end of circulation

char line[80];

instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "const", "do", "end", "true", "false",
	"if", "odd", "procedure", "then", "var", "while",
	"else", "elif", "for", "return", "exit", "switch",
	"case", "default", "break", "continue", "goto",
	"void", "int", "print"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, /*SYM_CALL,*/ SYM_CONST, SYM_DO, SYM_END,
	SYM_TRUE, SYM_FALSE, SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN,
	SYM_VAR, SYM_WHILE, SYM_ELSE, SYM_ELIF, SYM_FOR, SYM_RETURN,
	SYM_EXIT, SYM_SWITCH, SYM_CASE, SYM_DEFAULT, SYM_BREAK,
	SYM_CONTINUE, SYM_GOTO, SYM_VOID, SYM_INT, SYM_PRINT
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LSQUARE, SYM_RSQUARE, SYM_NOT
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '[', ']', '!'
};

#define MAXINS   20
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "LODI", "LODS", "LEA", "STO", "STOI", "STOS",
	"CAL", "CALS", "INT", "JMP", "JPC", "JND", "JNDN", "RET",
	"EXT", "JET", "PRNT"
};

struct type																						// added by nanahka 17-12-15
{
	int k; // kind of parameter (for procedure) / dimension (for array)
	struct type *ptr;
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	comtyp *ptr;							// modified by nanahka 17-12-15
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
	comtyp  *ptr;							// modified by nanahka 17-12-15
} mask;

typedef struct                                //added by lzp 17/12/14
{
	int t;                   //the condition of switch
	int c;                   //the index of first ins of every case
	int flag;                //to mark break
	int cx_bre;               //break cx
}casetab;
casetab *switchtab;
int tx_c = 0;
int maxcase = MAX_CASE;

typedef struct
{
	int ty;                     //type
	int c;                     //cx of the stat
} col;
col cltab[MAX_CONTROL];             //max depth of circulation
int cltop = 0;                      //top of cltab
int count = 0;                   //count num of break and continue

FILE* infile;

// EOF PL0.h
