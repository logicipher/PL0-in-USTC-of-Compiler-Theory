// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "pl0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

    // added & modified by nanahka 17-09-21 as labwork_1
    // an un-recursive method to eliminate repeated comments
    ////////////////////////////////////////////////
    // two kinds of comments supported
    // 1. line comment:     //...
    // 2. block comment:    /*...*/
    ////////////////////////////////////////////////
	do
    {
        while (ch == ' '||ch == '\t')
            getch();

        if (ch == '/')
        {
            getch();
            if (ch == '/')
            {
                cc = ll = 0;
                ch = ' ';
            }
            else if (ch == '*')
            {
                do
                {
                    ch = ' ';
                    while (ch != '*')
                        getch();
                    getch();
                }
                while (ch != '/');
                ch = ' ';
            }
            else
            {
                sym = SYM_SLASH;
                return;
            }
        }
    }
    while (ch == ' ');


	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			error(37); // '=' expected.				// modified by nanhka 17-11-13
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '[')		   // [			// added by nanahka 17-11-12
	{
		sym = SYM_LSQUARE;
		getch();
	}
	else if (ch == ']')		   // ]			// added by nanahka 17-11-12
	{
		sym = SYM_RSQUARE;
		getch();
	}
	else if(ch=='&')
	{
		getch();
		if(ch=='&')
		{
			sym=SYM_AND;
		        getch();
		}
	}
	else if(ch=='|')
	{
		getch();
		if(ch=='|')
			sym=SYM_OR;
		else
			error(48);     //missing a '|'
		getch();
	}
	else if(ch=='!')
	{
		sym=SYM_NOT;
		getch();
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
//
//			f			l			a
//			INT			--			numerical constant			// allocate storage in stack
//			LIT			--			numerical constant			// set a constant on top of the stack
//			LOD			levelDiff	data address
//			LODI		levelDiff	--(addr at top of the stack)
//			STO			levelDiff	target address
//			STOI		levelDiff	--(addr at top - 1 of the stack, data at top)
//			CAL			levelDiff	procedure address
//			JMP			--			procedure address
//			JPC			--			procedure address			// if stack[top]==0, jump to an address
//			OPR			--			type of algebraic/logical instructions
//
//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index		// accumulated offset in current AR

//////////////////////////////////////////////////////////////////////
//TABLE: (table[0] for error declaration, correct entries start from table[1])
//				name		kind			level		address						ptr
// const		id			enter(kind)		--------value(stored in TABLE)---------	--
// var			id			enter(kind)		level		dx							--
// procedure	id			enter(kind)		level		cx(set by block(), line752)	ptr[0] is the number of its parameters
// array		id			enter(kind)		level		dx ~ dx + ptr[0] - 1		ptr[1..ptr[0]] are volumes
//																					 of the dimensions
//*: Since all the entries created within a procedure will be released after the
//  declaration of the procedure, except for the name of the procedure itself, all
//  the entries in the table at a special moment are available for the statement
//  to be analyzed.
//////////////////////////////////////////////////////////////////////
// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	int position(char* id, int tx_beg);

	if (kind == ID_PROCEDURE && position(id, TABLE_BEGIN) ||									// added by nanahka 17-11-20
		kind != ID_PROCEDURE && position(id, tx_b))
	{
		error(31); // Redeclaration of an identifier.
		return;
	}

	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:					// modified by nanahka 17-11-20
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->ptr = (int*)malloc(sizeof(int));
		*mk->ptr = 0;
		break;
	case ID_ARRAY:						// added by nanahka 17-11-13
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx;
		int i = ptr[0], s = ptr[1];
		while (--i) {s *= ptr[ptr[0]- i + 1];}
		dx += s;
		mk->ptr = ptr;
		ptr = 0;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
// language feature: Check if there is already a same var/const name within
//   current block, or a procedure name in the whole TABLE. If NOT, return 0.
//   This feature is to prevent wrong input of ids within compiling process,
//   and support recursive calling of ancestor procedures.
int position(char* id, int tx_beg)																// modified by nanahka 17-11-20
{
	int i;
	i = tx + 1;
	if (tx_beg)
	{
		strcpy(table[0].name, table[tx_beg].name);
	}
	strcpy(table[tx_beg].name, id);
	while (strcmp(table[--i].name, id) != 0);
	if (tx_beg)
	{
		strcpy(table[tx_beg].name, table[0].name);
	}
	return tx_beg == i ? i - tx_beg : i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
int expression(symset fsys, symset ksys, int CONST_CHECK);
void vardeclaration(symset fsys, symset ksys)
{
	int con_expr;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)			// added & modified by nanahka 17-11-14
	{
		getsym();
		if (sym == SYM_LSQUARE)
		{ // array declaration
			char id_t[MAXIDLEN + 1];
			strcpy(id_t, id);
			int dim[MAXARYDIM + 1] = {};
			do
			{
				getsym();
				set = createset(SYM_RSQUARE, SYM_NULL);
				set1 = uniteset(ksys, set);
				con_expr = expression(set, set1, CONST_EXPR);
				destroyset(set);
				destroyset(set1);
				if (con_expr <= 0 || con_expr > MAXARYVOL)
				{
					error(36); // Volume of a dimension is out of range.
					break;
				}
				if ( (++dim[0]) > MAXARYDIM)
				{
					error(35); // There are too many dimensions.
					--dim[0];
					break;
				}
				dim[dim[0]] = con_expr;
				if (sym != SYM_RSQUARE)
				{
					error(34); // ']' expected.				// if ']' lost, go finding the next '['
				}
				else
				{
					getsym();
				}
			}
			while (sym == SYM_LSQUARE);
			if (dim[0])										// modified by nanahka 17-11-13
			{
				ptr = (int*)malloc( (dim[0] + 1) * sizeof(int));
				ptr[0] = dim[0]++;
				while (--dim[0]) {ptr[dim[0]] = dim[dim[0]];}
				strcpy(id, id_t);
				enter(ID_ARRAY);
			}
		}
		else
		{ // variable declaration
			enter(ID_VARIABLE);
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
int factor(symset fsys, symset ksys, int CONST_CHECK)
{
	int i, rv = 0;
	symset set, set1;

	test(fac_first_sys, ksys, 24); // The symbol can not be as the beginning of an expression.
											 // "factor" is unfamiliar for the user, thus "expression" used.
	if (inset(sym, fac_first_sys))																// modified by nanahka 17-11-14
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id, TABLE_BEGIN)) == 0)
			{
				error(11); // Undeclared identifier.
				getsym();
			}
			else
			{
				switch (table[i].kind)															// modified by nanahka 17-11-14
				{
					mask* mk;
				case ID_CONSTANT:
					if (CONST_CHECK)
					{ // UNCONST_EXPR
						gen(LIT, 0, table[i].value);
					}
					else
					{ // CONST_EXPR
						rv = table[i].value;
					}
					break;
				case ID_VARIABLE:
					if (CONST_CHECK)
					{ // UNCONST_EXPR
						mk = (mask*) &table[i];
						gen(LOD, level - mk->level, mk->address);
					}
					else
					{ // CONST_EXPR
						error(28); // Variables can not be in a const expression.
					}
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				case ID_ARRAY:																	// added by nanahka 17-11-15
					if (CONST_CHECK)
					{ // UNCONST_EXPR
						mk = (mask*) &table[i];
						getsym();
						if (sym != SYM_LSQUARE)
						{
							error(38); // '[' expected
							mk = 0;
						}
						else
						{
							getsym();
						}
						if (mk)
						{
							ptr = mk->ptr;
							int d = *ptr; // d <- dimensions of the array
							set = createset(SYM_RSQUARE, SYM_NULL);
							set1 = uniteset(ksys, set);
							expression(set, set1, UNCONST_EXPR);
							if (sym != SYM_RSQUARE)
							{
								error(34); // ']' expected.
							}
							else
							{
								getsym();
							}
							--d;
							while (sym == SYM_LSQUARE && d)
							{
								getsym(); // take '['
								gen(LIT, 0, ptr[ptr[0] - d + 1]);
								gen(OPR, 0, OPR_MUL);
								expression(set, set1, UNCONST_EXPR);
								gen(OPR, 0, OPR_ADD);
								if (sym != SYM_RSQUARE)
								{
									error(34); // ']' expected.
								}
								else
								{
									getsym();
								}
								--d;
							}
							destroyset(set);
							destroyset(set1);
							if (!d)
							{ // number of subscripts read == array dimensions
								//gen(LIT, 0, sizeof(int));
								//gen(OPR, 0, OPR_MUL);
								gen(LIT, 0, mk->address);
								gen(OPR, 0, OPR_ADD);
							}
							else
							{ // number of subscripts read < array dimensions
								error(29); // Too few subscripts.
								i = 0;
							}
						} // if
						if (!mk)
						{ // discard the leftover parts of the subscripts
							test(ksys, ksys, 19); // Incorrect symbol.
						}
						else																	// added by nanahka 17-11-15
						{
							if (sym == SYM_LSQUARE)
							{
								test(ksys, ksys, 30); // Too many subscripts.
							}
							gen(LODI, level - mk->level, 0);
						}
					}
					else
					{ // CONST_EXPR
						error(28); // Variables can not be in a const expression.
					}
					break;
				} // switch
				if (table[i].kind != ID_ARRAY)
				{
					getsym();
					if (sym == SYM_LSQUARE)
					{
						error(27); // Applying the subscripts operator on non-array.
						getsym();
					}
					test(ksys, ksys, 23); // The symbol can not be followed by a factor.
				}
			} // if
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(LIT, 0, num);
			}
			else
			{ // CONST_EXPR
				rv = num;
			}
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = createset(SYM_RPAREN, SYM_NULL);
			set1 = uniteset(ksys, set);
			rv = expression(set, set1, CONST_CHECK);											// modified by nanahka 17-11-14
			destroyset(set);
			destroyset(set1);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			rv = factor(fsys, ksys, CONST_CHECK);												// modified by nanahka 17-11-14
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(OPR, 0, OPR_NEG);
			}
			else
			{ // CONST_EXPR
				rv = -rv;
			}
		}
		test(ksys, ksys, 23); // The symbol can not be followed by a factor.
	} // if
	return rv;																					// added by nanahka 17-11-14
} // factor

//////////////////////////////////////////////////////////////////////
int term(symset fsys, symset ksys, int CONST_CHECK)
{
	int mulop, r, rv = 0;
	symset set, set1;

	set = expandset(fsys, SYM_TIMES, SYM_SLASH, SYM_NULL);
	set1 = expandset(ksys, SYM_TIMES, SYM_SLASH, SYM_NULL);

	rv = factor(set, set1, CONST_CHECK);														// modified by nanahka 17-11-14
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		r = factor(set, set1, CONST_CHECK);														// modified by nanahka 17-11-14
		if (mulop == SYM_TIMES)
		{
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(OPR, 0, OPR_MUL);
			}
			else
			{ // CONST_EXPR
				rv *= r;
			}
		}
		else
		{
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(OPR, 0, OPR_DIV);
			}
			else
			{ // CONST_EXPR
				rv /= r;
			}
		}
	} // while
	destroyset(set);
	destroyset(set1);
	return rv;
} // term

//////////////////////////////////////////////////////////////////////
int expression(symset fsys, symset ksys, int CONST_CHECK)
{
	int addop, r, rv = 0;
	symset set, set1;

	set = expandset(fsys, SYM_PLUS, SYM_MINUS, SYM_NULL);
	set1 = expandset(ksys, SYM_PLUS, SYM_MINUS, SYM_NULL);

	rv = term(set, set1, CONST_CHECK);															// modified by nanahka 17-11-14
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		r = term(set, set1, CONST_CHECK);														// modified by nanahka 17-11-14
		if (addop == SYM_PLUS)
		{
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(OPR, 0, OPR_ADD);
			}
			else
			{ // CONST_EXPR
				rv += r;
			}
		}
		else
		{
			if (CONST_CHECK)
			{ // UNCONST_EXPR
				gen(OPR, 0, OPR_MIN);
			}
			else
			{ // CONST_EXPR
				rv -= r;
			}
		}
	} // while
	destroyset(set);
	destroyset(set1);
	return rv;
} // expression
void rel_expr(symset fsys) //condition
{
	int relop;
	symset set;

	/* //2017-0924
	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	*/
	{
		set = uniteset(relset, fsys);
		expression(set);
		//destroyset(set);
		if (!inset(sym, relset))
		{
			//error(20);//2017-09-24
		}
		else //if (inset(sym, relset))
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
		destroyset(set);
	} // else
} // rel_expr 


void and_expr(symset fsys)
{
	symset set;

	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));

	rel_expr(set);
	while (sym == SYM_AND)
	{
		getsym();
		rel_expr(set);
		gen(OPR, 0, OPR_AND);

	} // while

	destroyset(set);
}
//////////////////////////////////////////////////////////////////////
void or_expr(symset fsys)
{
	symset set;

	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));

	and_expr(set);
	while (sym == SYM_OR)
	{
		getsym();
		and_expr(set);
		gen(OPR, 0, OPR_OR);

	} // while
	destroyset(set);
} // or_expr
///////////////////////////////////////////////////////
void not_expr(symset fsys)
{
  symset set;
  set=uniteset(fsys,createset(SYM_NOT,SYM_NULL));
  not_expr(set);
  while(sym==SYM_NOT)
{  
   getsym();
   and_expr(set);
   gen(OPR,0,OPR_NOT);
}
   destoryset(set);
}
  

//////////////////////////////////////////////////////////////////////
void condition(symset fsys, symset ksys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys, ksys, UNCONST_EXPR);													// modified by nanahka 17-11-13
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, ksys);
		expression(relset, set, UNCONST_EXPR);													// modified by nanahka 17-11-13
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys, ksys, UNCONST_EXPR);												// modified by nanahka 17-11-13
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition


///////////////////////////////////////////////////////////
void ex_condition(symset fsys)
{	int relop;
	if (sym==SYM_NOT)//条件第一个是NOT
	{ 	getsym();
		condition(fsys);
		gen(OPR,0,OPR_NOT);
		}
	else{condition(fsys);}
	if(sym ==SYM_THEN||sym==SYM_SEMICOLON)
	{false_out[condition_level][false_count[condition_level]++]=cx;
	gen(JPC,0,0);//条件不成立
	true_out[condition_level][true_count[condition_level]++]=cx;
	gen(JMP,0,0);//条件成立
	return;
	}
	while(sym==SYM_AND||sym==SYM_OR||sym==SYM_NOT)
	{	
	relop=sym;
	getsym();
	switch (relop)
	{
	case SYM_OR:
		true_out[condition_level][true_count[condition_level]++]=cx;
		gen(JMP,0,0);//如果成立，调到true的出口，等待回填
		if(sym==SYM_NOT)//or的后面是not
		{
		getsym();
		condition(fsys);
		gen(OPR,0,OPR_NOT);//布尔值取反
		}
		else
		{condition(fsys);}
		if(sym==SYM_THEN||sym==SYM_SEMICOLON)
		{false_out[condition_level][false_count[condition_level]++]=cx;//条件不成立
		gen(JPC,0,0);
		true_out[condition_level][true_count[condition_level]++]=cx;//条件成立
		gen(JMP,0,0);
		return;
	}
	break;
	case SYM_AND:
		false_out[condition_level][false_count[condition_level]++]=cx;
		gen(JMP,0,0);//跳到false出口
		if(sym==SYM_NOT)//and的后面是not
		{
		getsym();
		condition(fsys);
		gen(OPR,0,OPR_NOT);//布尔值取反
		}
		else
		{condition(fsys);}
		if(sym==SYM_THEN||sym==SYM_SEMICOLON)
		{false_out[condition_level][false_count[condition_level]++]=cx;//条件不成立
		gen(JPC,0,0);
		true_out[condition_level][true_count[condition_level]++]=cx;//条件成立
		gen(JMP,0,0);
		return;}
		
	break;
	}
}
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys, symset ksys)
{
	int i, cx1, cx2,cx3,cx4;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{
		mask* mk;
		if (! (i = position(id, TABLE_BEGIN)))
		{
			error(11); // Undeclared identifier.
			getsym();
		}
		else if (table[i].kind == ID_PROCEDURE)
		{ // procedure call
			getsym();
			mk = (mask*) &table[i];
			int n = *mk->ptr;
			if (sym == SYM_LPAREN)
			{
				getsym();
				set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
				set1 = uniteset_mul(ksys, set, exp_first_sys, 0);
				if (n)
				{
					if (sym == SYM_RPAREN)
					{
						error(42); // Too few parameters in a procedure.
					}
					else
					{
						test(exp_first_sys, set1, 24); // The symbol can not be as the beginning of an expression.
					}
				}
				while (inset(sym, exp_first_sys) && n--)
				{
					expression(set, set1, UNCONST_EXPR);
					if (sym == SYM_COMMA)
					{
						getsym();
					}
					else if (sym == SYM_RPAREN)
					{
						getsym();
						if (n)
						{
							error(42); // Too few parameters in a procedure.
						}
						break;
					}
					else
					{
						error(40); // Missing ',' or ')'.
					}
				} // while
				destroyset(set);
				destroyset(set1);
				if (inset(sym, exp_first_sys))
				{
					error(39); // Too many parameters in a procedure.
				}
				if (sym == SYM_RPAREN)
				{
					getsym();
				}
			} // if
			if (!n)
			{
				gen(CAL, level - mk->level, mk->address);
			}
		}
		else
		{ // variable/array assignment
			if (table[i].kind == ID_ARRAY)
			{
				getsym();
				if (sym != SYM_LSQUARE)
				{
					error(38); // '[' expected
					i = 0;
				}
				else
				{
					getsym();
				}
				if (i)
				{
					mk = (mask*) &table[i];
					ptr = mk->ptr;
					int d = *ptr; // d <- dimensions of the array
					set = createset(SYM_RSQUARE, SYM_NULL);
					set1 = uniteset(ksys, set);
					expression(set, set1, UNCONST_EXPR);
					if (sym != SYM_RSQUARE)
					{
						error(34); // ']' expected.
					}
					else
					{
						getsym();
					}
					--d;
					while (sym == SYM_LSQUARE && d)
					{
						getsym(); // take '['
						gen(LIT, 0, ptr[ptr[0] - d + 1]);
						gen(OPR, 0, OPR_MUL);
						expression(set, set1, UNCONST_EXPR);
						gen(OPR, 0, OPR_ADD);
						if (sym != SYM_RSQUARE)
						{
							error(34); // ']' expected.
						}
						else
						{
							getsym();
						}
						--d;
					}
					destroyset(set);
					destroyset(set1);
					if (!d)
					{ // number of subscripts read == array dimensions
						//gen(LIT, 0, sizeof(int));
						//gen(OPR, 0, OPR_MUL);
						gen(LIT, 0, mk->address);
						gen(OPR, 0, OPR_ADD);
					}
					else
					{ // number of subscripts read < array dimensions
						error(29); // Too few subscripts.
						i = 0;
					}
				} // if
				set = createset(SYM_BECOMES, SYM_NULL);
				set1 = uniteset(ksys, set);
				if (!i)
				{ // discard the leftover parts of the subscripts
					test(set, set1, 19); // Incorrect symbol.
				}
				else if (sym == SYM_LSQUARE)														// added by nanahka 17-11-15
				{
					test(set, set1, 30); // Too many subscripts.
				}
				destroyset(set);
				destroyset(set1);
			}
			else if (table[i].kind != ID_VARIABLE)													// modified by nanahka 17-11-14
			{
				error(12); // Illegal assignment.
				i = 0;
				getsym();
			}
			else
			{ // ID_VARIABLE
				getsym();
			}
			if (sym == SYM_LSQUARE)
			{ // ID_VARIABLE / Illegal / Too many subscripts in array
				error(27); // Applying the subscripts operator on non-array.
				getsym();
				set = createset(SYM_BECOMES, SYM_NULL);
				set1 = uniteset(ksys, set);
				test(set, set1, 19); // Incorrect symbol.
				destroyset(set);
				destroyset(set1);
			}
			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}
			expression(fsys, ksys, UNCONST_EXPR);													// modified by nanahka 17-11-13
			mk = (mask*) &table[i];
			if (i)
			{
				if (table[i].kind == ID_VARIABLE)
				{
					gen(STO, level - mk->level, mk->address);
				}
				else // ID_ARRAY
				{
					gen(STOI, level - mk->level, 0);
				}
			}
		} // if
	}
/*	else if (sym == SYM_CALL)
	{ // procedure call
		;
	}*/
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_NULL);
		set = uniteset_mul(ksys, set1, stat_first_sys, 0);
		condition(set1, set);																	// modified by nanahka 17-11-13
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys, ksys);	// modified by nanahka 17-11-13
		cx2=cx;
		gen(JMP,0,0);
		code[cx1].a = cx;
		if(sym!=SYM_ELSE)
			error(47);      //missing 'else
		statement(fsys,ksys);
		code[cx2].a=cx;
	} //PL0 has been changed to 'else' is necessary,but I will find a way to change it to C_style
		
			
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset_mul(ksys, set1, stat_first_sys, 0);										// modified by nanahka 17-11-13
		setinsert(set, SYM_END);
		do
		{
			statement(set1, set);
			if (sym == SYM_SEMICOLON)															// modified by nanahka 17-11-13
			{
				getsym();
			}
			else
			{
				error(10); // ';' expected.
			}
		}
		while (inset(sym, stat_first_sys));
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // 'end' expected.														// modified by nanahka 17-11-13
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset_mul(ksys, set1, stat_first_sys, 0);
		condition(set1, set);																	// modified by nanahka 17-11-13
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys, ksys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if(sym=SYM_FOR)
	{//for statement
		getsym();
		if(sym!=SYM_LPAREN)
			error(43);   //missing '('
		getsym();
		if((i=position(id))==0)
			error(11);           //id not declared
		if(table[i].kind!=ID_VARIABLE)
			error(44);           //it must be a variable
		set1=createset(SYM_SEMICOLON,SYM_NULL);
		set=uniteset_mul(ksys,set1,SYM_IDENTIFIER,0);
		expression(set1,set);
		//destroyset(set1);
		//destoryset(set);
		if(sym!=SYM_SEMICOLON)
			error(10);            //';' expected
		getsym();
		cx1=cx;
		condition(set1,set);          //condition
		destroyset(set);
		destroyset(set1);
		cx2=cx;
		gen(JPC,0,0);
		cx3=cx;
		gen(JMP,0,0);
		if((i=position(id))==0)
			error(11);           //id not declared
		if(table[i].kind!=ID_VARIABLE)
			error(44);           //it must be a variable
		set1=createset(SYM_RPAREN,SYM_NULL);
		set=uniteset_mul(ksys,set1,stat_firat_sys,0);
		cx4=cx;
		expression(set1,set);        //change cycle var  
		gen(JMP,0,cx1);
		code[cx3].a=cx;
		statement(fsys,ksys);       //body of 'for'
		gen(JMP,0,cx4);
		code[cx2].a=cx;
	}
	else if(sym==SYM_RETURN)
	{
		getsym();
		set1=createset(SYM_SEMICOLON,SYM_NULL);
		set=uniteset_mul(ksys,set1,stat_first_sys,SYM_SEMICOLON,0);
		expression(set1,set);
		if(sym!=SYM_SEMICOLON)
			error(26);          //missing ';'
		gen(OPR,0,OPR_RET);
		cx_ret[i_ret]=cx;
		gen(JMP,0,0);
	}
	else if(sym==SYM_EXIT)
	{
		getsym();
		if(sym!=SYMLPAREN)
			error(43);         //'(' is needed
		getsym();
		i=position(id);
		if(table[i].kind!=ID_CONSTANT)
			error(45);          //'exit' have to return a constant
		getsym();
		if(sym!=SYM_LPAREN)
			error(22);           //missing ')'
		getsym();
		if(sym!=SYM_SEMICOLON)
			error(10);           //missing ';'
		gen(LIT,0,table[i].value);
		gen(OPR,0,OPR_RET);
		if(i_exit=MAX_EXIT)
			error(46);          //this is the maximum of 'exit'
		cx_exit[i_exit++]=cx;
		gen(JMP,0,0);
	}
		
		
	test(ksys, ksys, 19);																		// modified by nanahka 17-11-20
} // statement

//////////////////////////////////////////////////////////////////////
void block(symset fsys, symset ksys)	// fsys/ksys is the Follow/KeyWord set of current block
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int block_tx = tx_b;																// added by nanahka 17-11-20
	int savedTx, savedDx;																// added by nanahka 17-11-20
	symset set1, set;
	int i=0;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[tx_b];	// when block() called after creating a procedure, this is the entry of the procedure
	mk->address = cx;			// enter index of the next code JMP in "address" of the procedure
	gen(JMP, 0, 0);				// generate an instruction to jump to the procedure, the address filled later
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		tx_b = block_tx; // set tx_b to the beginning index of current block;					// added by nanahka 17-11-20
		if (sym == SYM_CONST)		//WARNING: other types of declaration precede the procedure!!! or tx_b will be reset!
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do																					// modified by nanahka 17-11-14
			{
				set = createset(SYM_COMMA, SYM_SEMICOLON, SYM_NULL);
				set1 = uniteset_mul(ksys, set, blk_first_sys, 0);
				vardeclaration(set, set1);
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration(set, set1);
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
				destroyset(set);
				destroyset(set1);
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			int PROC_CREATED = TRUE;
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
				PROC_CREATED = FALSE;
			}
			level++;																			// modified by nanahka 17-11-20
			savedTx = tx_b = tx; // entry of the procedure created(if succeeded)
			if (sym == SYM_LPAREN)
			{
				getsym();
				savedDx = dx; // n parameters bound to dx -n-1,-n,...,-1. The actual dx doesn't increase.
				int n = 0;
				while (sym == SYM_IDENTIFIER)
				{
					char id_t[MAXIDLEN + 1];
					strcpy(id_t, id);
					getsym();
					set = createset(SYM_COMMA, SYM_RPAREN, SYM_LSQUARE, SYM_NULL);
					set1 = uniteset_mul(ksys, set, blk_first_sys, 0);
					setinsert_mul(set1, SYM_IDENTIFIER, SYM_SEMICOLON, SYM_NULL);
					test(set, set1, 40); // Missing ',' or ')'.
					destroyset(set);
					destroyset(set1);
					if (sym == SYM_LSQUARE)
					{
						error(41); // Array type as parameter forbidden.
						set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
						set1 = uniteset_mul(ksys, set, blk_first_sys, 0);
						setinsert_mul(set1, SYM_IDENTIFIER, SYM_SEMICOLON, SYM_NULL);
						test(set, set1, 40); // Missing ',' or ')'.
						destroyset(set);
						destroyset(set1);
					}
					else
					{
						if ( (++n) > MAXFUNPMT)
						{
							error(39); // Too many parameters in a procedure.
							--n;
							break;
						}
						strcpy(id, id_t);
						dx = 0;				// as a sign if backpatching fails
						enter(ID_VARIABLE); // address will entered later
					}
					if (sym == SYM_COMMA)
					{
						getsym();
					}
					else if (sym == SYM_RPAREN)
					{
						break;
					}
				} // while
				if (sym == SYM_RPAREN)
				{
					getsym();
				}
				dx = savedDx;
				if (PROC_CREATED)
				{
					*table[tx_b].ptr = n;
					while (n--)
					{
						mask *mk = (mask*)&table[tx - n];
						mk->address = -n - 1;
					}
				}
			}
			else
			{
				*table[tx_b].ptr = 0;
			}

			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset_mul(ksys, set1, blk_first_sys, 0);
			test(set1, set, 26); // Missing ';'.
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			block(set1, set);																	// modified by nanahka 17-11-13
			destroyset(set1);
			destroyset(set);
			tx = savedTx;	// all the codes of the procedure generated, release the entries in TABLE
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(26); // Missing ';'.														// modified by nanahka 17-11-13
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		//set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(blk_first_sys, ksys);
		test(blk_first_sys, set, 7); // Declaration/Statement expected.							// modified by nanahka 17-11-13
		//destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, decl_first_sys));

	code[mk->address].a = cx;	// fill the address of JMP to the procedure(no code belonging to it generated during declaration)
	mk->address = cx;			// change the "address" attribute of the procedure to its true entrance, save 1 JMP
	cx0 = cx;
	gen(INT, 0, block_dx);		// distribute storage for Static Link(<-bp), Dynamic Link, Return Address, and variables
	//set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	//set = uniteset(fsys, ksys);
	statement(fsys, ksys);	// modified by nanahka 17-11-13
	
	//destroyset(set1);
	//destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	for(cx_ret[i_ret]!=0)
	{
		code[cx_ret[i]].a=cx;
		cx_ret[i++]=0;
	}
	i=0;
	while(cx_exit[i++]!=0)
	{                                           //all 'exit' gen a instruction to jump to this end
		code[cx_exit[i-1]]=cx;
	}
	test(ksys, ksys, 8); // Follow the statement is an incorrect symbol.						// modified by nanahka 17-11-20
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case LODI:
			stack[top] = stack[base(stack, b, i.l) + stack[top]];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case STOI:
			stack[base(stack, b, i.l) + stack[top - 1]] = stack[top];
			printf("%d\n", stack[top]);
			top -= 2;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
int main ()
{
	FILE* hbin;
	char s[80];
	int i;
	//symset set, set1, set2;																	// modified by nanahka 17-11-14

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	 // create First/Follow/KeyWord symbol sets													// modified by nanahka 17-11-13
	phi 				= createset(SYM_NULL);
	relset 				= createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	decl_first_sys 		= createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	stat_first_sys 		= createset(SYM_IDENTIFIER, SYM_BEGIN, /*SYM_CALL,*/ SYM_IF, SYM_WHILE, SYM_NULL);	// deleted by nanahka 17-11-20
	blk_first_sys 		= uniteset(decl_first_sys, stat_first_sys);
	fac_first_sys 		= createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);
	exp_first_sys		= fac_first_sys;

	main_blk_follow_sys = createset(SYM_PERIOD, SYM_NULL);


	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

//	set1 = createset(SYM_PERIOD, SYM_NULL);
//	set2 = uniteset(declbegsys, statbegsys);
//	set = uniteset(set1, set2);
	block(main_blk_follow_sys, main_blk_follow_sys);
//	destroyset(set1);
//	destroyset(set2);
//	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(decl_first_sys);
	destroyset(stat_first_sys);
	destroyset(blk_first_sys);
	destroyset(fac_first_sys);
	//destroyset(exp_first_sys);
	destroyset(main_blk_follow_sys);
//	destroyset(declbegsys);
//	destroyset(statbegsys);
//	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);

	getchar();
	getchar();
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
