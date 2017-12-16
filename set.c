

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "set.h"

symset uniteset(symset s1, symset s2)
{
	symset s;
	snode* p;

	s1 = s1->next;
	s2 = s2->next;

	s = p = (snode*) malloc(sizeof(snode));
	while (s1 && s2)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		if (s1->elem < s2->elem)
		{
			p->elem = s1->elem;
			s1 = s1->next;
		}
		else if (s1->elem > s2->elem)															// modified by nanahka 17-11-20
		{
			p->elem = s2->elem;
			s2 = s2->next;
		}
		else
		{
			p->elem = s1->elem;
			s1 = s1->next;
			s2 = s2->next;
		}
	}

	while (s1)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;

	}

	while (s2)
	{
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}

	p->next = NULL;

	return s;
} // uniteset

symset uniteset_mul(symset s1, .../* NULL */)													// added by nanahka 17-11-13
{
	va_list list;
	symset s = s1;

	va_start(list, s1);
	s1 = va_arg(list, symset);

	while (s1)
	{
		s = uniteset(s, s1);
		s1 = va_arg(list, symset);
	}
	va_end(list);
	return s;
}

void setinsert(symset s, int elem)
{
	snode* p = s;
	snode* q;

	while (p->next && p->next->elem < elem)
	{
		p = p->next;
	}

	if (p->next && p->next->elem == elem)														// added by nanahka 17-11-20
	{
		return;
	}

	q = (snode*) malloc(sizeof(snode));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
} // setinsert

void setinsert_mul(symset s, .../* SYM_NULL */)													// added by nanahka 17-11-20
{
	va_list list;
	int elem;

	va_start(list, s);
	elem = va_arg(list, int);

	while (elem)
	{
		setinsert(s, elem);
		elem = va_arg(list, int);
	}
	va_end(list);
}

symset expandset(symset s, .../* SYM_NULL */)													// added by nanahka 17-11-13
{
	va_list list;
	symset p;
	int elem;

	p = (snode*) malloc(sizeof(snode));
	p->next = NULL;

	va_start(list, s);
	elem = va_arg(list, int);

	while (elem)
	{
		setinsert(p, elem);
		elem = va_arg(list, int);
	}
	va_end(list);

	return uniteset(s, p);
} // expandset

void deleteset(symset s, .../* SYM_NULL */)														// added by nanahka 17-11-21
{
	va_list list;
	symset p;
	int elem;

	p = (snode*) malloc(sizeof(snode));
	p->next = NULL;

	va_start(list, s);
	elem = va_arg(list, int);
	while (elem)
	{
		setinsert(p, elem);
		elem = va_arg(list, int);
	}

	snode *s0, *q;
	s0 = s;
	s = s->next;
	q = p->next;

	while (s && q)
	{
		if (s->elem == q->elem)
		{
			s0->next = s->next;
			s->next = NULL;
			destroyset(s);
			s = s0->next;
			q = q->next;
		}
		else if (s->elem > q->elem)
		{
			q = q->next;
		}
		else
		{
			s0 = s;
			s = s->next;
		}
	}
	destroyset(p);
}

symset createset(int elem, .../* SYM_NULL */)
{
	va_list list;
	symset s;

	s = (snode*) malloc(sizeof(snode));
	s->next = NULL;

	va_start(list, elem);
	while (elem)
	{
		setinsert(s, elem);
		elem = va_arg(list, int);
	}
	va_end(list);
	return s;
} // createset

void destroyset(symset s)
{
	snode* p;

	while (s)
	{
		p = s;
		p->elem = -1000000;
		s = s->next;
		free(p);
	}
} // destroyset

int inset(int elem, symset s)
{
	s = s->next;
	while (s && s->elem < elem)
		s = s->next;

	if (s && s->elem == elem)
		return 1;
	else
		return 0;
} // inset

// EOF set.c
