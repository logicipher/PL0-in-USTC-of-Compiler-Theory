#ifndef SET_H
#define SET_H

typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;			// elem of head node not used

symset phi, 														// added by nanahka 17-11-13
	   blk_first_sys, decl_first_sys, stat_first_sys,
	   fac_first_sys,
	   main_blk_follow_sys,
	   relset;

symset createset(int data, .../* SYM_NULL */);
void destroyset(symset s);
symset uniteset(symset s1, symset s2);
int inset(int elem, symset s);

#endif
// EOF set.h
