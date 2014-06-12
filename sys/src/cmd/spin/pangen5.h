/***** spin: pangen5.h *****/

/* Copyright (c) 1997-2003 by Lucent Technologies, Bell Laboratories.     */
/* All Rights Reserved.  This software is for educational purposes only.  */
/* No guarantee whatsoever is expressed or implied by the distribution of */
/* this code.  Permission is given to distribute this code provided that  */
/* this introductory message is not removed and no monies are exchanged.  */
/* Software written by Gerard J. Holzmann.  For tool documentation see:   */
/*             http://spinroot.com/                                       */
/* Send all bug-reports and/or questions to: bugs@spinroot.com            */

static char *Xpt[] = {
	"#if defined(MA) && (defined(W_XPT) || defined(R_XPT))",
	"static Vertex	**temptree;",
	"static char	wbuf[4096];",
	"static int	WCNT = 4096, wcnt=0;",
	"static uchar	stacker[MA+1];",
	"static ulong	stackcnt = 0;",
	"extern double	nstates, nlinks, truncs, truncs2;",
	"",
	"static void",
	"xwrite(int fd, char *b, int n)",
	"{",
	"	if (wcnt+n >= 4096)",
	"	{	write(fd, wbuf, wcnt);",
	"		wcnt = 0;",
	"	}",
	"	memcpy(&wbuf[wcnt], b, n);",
	"	wcnt += n;",
	"}",
	"",
	"static void",
	"wclose(fd)",
	"{",
	"	if (wcnt > 0)",
	"		write(fd, wbuf, wcnt);",
	"	wcnt = 0;",
	"	close(fd);",
	"}",
	"",
	"static void",
	"w_vertex(int fd, Vertex *v)",
	"{	char t[3]; int i; Edge *e;",
	"",
	"	xwrite(fd, (char *) &v,  sizeof(Vertex *));",
	"	t[0] = 0;",
	"	for (i = 0; i < 2; i++)",
	"		if (v->dst[i])",
	"		{	t[1] = v->from[i], t[2] = v->to[i];",
	"			xwrite(fd, t, 3);",
	"			xwrite(fd, (char *) &(v->dst[i]), sizeof(Vertex *));",
	"		}",
	"	for (e = v->Succ; e; e = e->Nxt)",
	"	{	t[1] = e->From, t[2] = e->To;",
	"		xwrite(fd, t, 3);",
	"		xwrite(fd, (char *) &(e->Dst), sizeof(Vertex *));",
	"",
	"		if (e->s)",
	"		{	t[1] = t[2] = e->S;",
	"			xwrite(fd, t, 3);",
	"			xwrite(fd, (char *) &(e->Dst), sizeof(Vertex *));",
	"	}	}",
	"}",
	"",
	"static void",
	"w_layer(int fd, Vertex *v)",
	"{	uchar c=1;",
	"",
	"	if (!v) return;",
	"	xwrite(fd, (char *) &c, 1);",
	"	w_vertex(fd, v);",
	"	w_layer(fd, v->lnk);",
	"	w_layer(fd, v->left);",
	"	w_layer(fd, v->right);",
	"}",
	"",
	"void",
	"w_xpoint(void)",
	"{	int fd; char nm[64];",
	"	int i, j;  uchar c;",
	"	static uchar xwarned = 0;",
	"",
	"	sprintf(nm, \"%%s.xpt\", Source);",
	"	if ((fd = creat(nm, 0666)) <= 0)",
	"	if (!xwarned)",
	"	{	xwarned = 1;",
	"		printf(\"cannot creat checkpoint file\\n\");",
	"		return;",
	"	}",
	"	xwrite(fd, (char *) &nstates, sizeof(double));",
	"	xwrite(fd, (char *) &truncs, sizeof(double));",
	"	xwrite(fd, (char *) &truncs2, sizeof(double));",
	"	xwrite(fd, (char *) &nlinks, sizeof(double));",
	"	xwrite(fd, (char *) &dfa_depth, sizeof(int));",
	"	xwrite(fd, (char *) &R,  sizeof(Vertex *));",
	"	xwrite(fd, (char *) &F,  sizeof(Vertex *));",
	"	xwrite(fd, (char *) &NF, sizeof(Vertex *));",
	"",
	"	for (j = 0; j < TWIDTH; j++)",
	"	for (i = 0; i < dfa_depth+1; i++)",
	"	{	w_layer(fd, layers[i*TWIDTH+j]);",
	"		c = 2; xwrite(fd, (char *) &c, 1);",
	"	}",
	"	wclose(fd);",
	"}",
	"",
	"static void",
	"xread(int fd, char *b, int n)",
	"{	int m = wcnt; int delta = 0;",
	"	if (m < n)",
	"	{	if (m > 0) memcpy(b, &wbuf[WCNT-m], m);",
	"		delta = m;",
	"		WCNT = wcnt = read(fd, wbuf, 4096);",
	"		if (wcnt < n-m)",
	"			Uerror(\"xread failed -- insufficient data\");",
	"		n -= m;",
	"	}",
	"	memcpy(&b[delta], &wbuf[WCNT-wcnt], n);",
	"	wcnt -= n;",
	"}",
	"",
	"static void",
	"x_cleanup(Vertex *c)",
	"{	Edge *e;	/* remove the tree and edges from c */",
	"	if (!c) return;",
	"	for (e = c->Succ; e; e = e->Nxt)",
	"		x_cleanup(e->Dst);",
	"	recyc_vertex(c);",
	"}",
	"",
	"static void",
	"x_remove(void)",
	"{	Vertex *tmp; int i, s;",
	"	int r, j;",
	"	/* double-check: */",
	"	stacker[dfa_depth-1] = 0; r = dfa_store(stacker);",
	"	stacker[dfa_depth-1] = 4; j = dfa_member(dfa_depth-1);",
	"	if (r != 1 || j != 0)",
	"	{	printf(\"%%d: \", stackcnt);",
	"		for (i = 0; i < dfa_depth; i++)",
	"			printf(\"%%d,\", stacker[i]);",
	"		printf(\" -- not a stackstate <o:%%d,4:%%d>\\n\", r, j);",
	"		return;",
	"	}",
	"	stacker[dfa_depth-1] = 1;",
	"	s = dfa_member(dfa_depth-1);",
	"",
	"	{ tmp = F; F = NF; NF = tmp; }	/* complement */",
	"		if (s) dfa_store(stacker);",
	"		stacker[dfa_depth-1] = 0;",
	"		dfa_store(stacker);",
	"		stackcnt++;",
	"	{ tmp = F; F = NF; NF = tmp; }",
	"}",
	"",
	"static void",
	"x_rm_stack(Vertex *t, int k)",
	"{	int j; Edge *e;",
	"",
	"	if (k == 0)",
	"	{	x_remove();",
	"		return;",
	"	}",
	"	if (t)",
	"	for (e = t->Succ; e; e = e->Nxt)",
	"	{	for (j = e->From; j <= (int) e->To; j++)",
	"		{	stacker[k] = (uchar) j;",
	"			x_rm_stack(e->Dst, k-1);",
	"		}",
	"		if (e->s)",
	"		{	stacker[k] = e->S;",
	"			x_rm_stack(e->Dst, k-1);",
	"	}	}",
	"}",
	"",
	"static Vertex *",
	"insert_withkey(Vertex *v, int L)",
	"{	Vertex *new, *t = temptree[L];",
	"",
	"	if (!t) { temptree[L] = v; return v; }",
	"	t = splay(v->key, t);",
	"	if (v->key < t->key)",
	"	{	new = v;",
	"		new->left = t->left;",
	"		new->right = t;",
	"		t->left = (Vertex *) 0;",
	"	} else if (v->key > t->key)",
	"	{	new = v;",
	"		new->right = t->right;",
	"		new->left = t;",
	"		t->right = (Vertex *) 0;",
	"	} else",
	"	{	if (t != R && t != F && t != NF)",
	"			Uerror(\"double insert, bad checkpoint data\");",
	"		else",
	"		{	recyc_vertex(v);",
	"			new = t;",
	"	}	}",
	"	temptree[L] = new;",
	"",
	"	return new;",
	"}",
	"",
	"static Vertex *",
	"find_withkey(Vertex *v, int L)",
	"{	Vertex *t = temptree[L];",
	"	if (t)",
	"	{	temptree[L] = t = splay((ulong) v, t);",
	"		if (t->key == (ulong) v)",
	"			return t;",
	"	}",
	"	Uerror(\"not found error, bad checkpoint data\");",
	"	return (Vertex *) 0;",
	"}",
	"",
	"void",
	"r_layer(int fd, int n)",
	"{	Vertex *v;",
	"	Edge *e;",
	"	char c, t[2];",
	"",
	"	for (;;)",
	"	{	xread(fd, &c, 1);",
	"		if (c == 2) break;",
	"		if (c == 1)",
	"		{	v = new_vertex();",
	"			xread(fd, (char *) &(v->key), sizeof(Vertex *));",
	"			v = insert_withkey(v, n);",
	"		} else	/* c == 0 */",
	"		{	e = new_edge((Vertex *) 0);",
	"			xread(fd, t, 2);",
	"			e->From = t[0];",
	"			e->To = t[1];",
	"			xread(fd, (char *) &(e->Dst), sizeof(Vertex *));",
	"			insert_edge(v, e);",
	"	}	}",
	"}",
	"",
	"static void",
	"v_fix(Vertex *t, int nr)",
	"{	int i; Edge *e;",
	"",
	"	if (!t) return;",
	"",
	"	for (i = 0; i < 2; i++)",
	"	if (t->dst[i])",
	"		t->dst[i] = find_withkey(t->dst[i], nr);",
	"",
	"	for (e = t->Succ; e; e = e->Nxt)",
	"		e->Dst = find_withkey(e->Dst, nr);",
	"		",
	"	v_fix(t->left, nr);",
	"	v_fix(t->right, nr);",
	"}",
	"",
	"static void",
	"v_insert(Vertex *t, int nr)",
	"{	Edge *e; int i;",
	"",
	"	if (!t) return;",
	"	v_insert(t->left, nr);",
	"	v_insert(t->right, nr);",
	"",
	"	/* remove only leafs from temptree */",
	"	t->left = t->right = t->lnk = (Vertex *) 0;",
	"	insert_it(t, nr);	/* into layers */",
	"	for (i = 0; i < 2; i++)",
	"		if (t->dst[i])",
	"			t->dst[i]->num += (t->to[i] - t->from[i] + 1);",
	"	for (e = t->Succ; e; e = e->Nxt)",
	"		e->Dst->num += (e->To - e->From + 1 + e->s);",
	"}",
	"",
	"static void",
	"x_fixup(void)",
	"{	int i;",
	"",
	"	for (i = 0; i < dfa_depth; i++)",
	"		v_fix(temptree[i], (i+1));",
	"",
	"	for (i = dfa_depth; i >= 0; i--)",
	"		v_insert(temptree[i], i);",
	"}",
	"",
	"static Vertex *",
	"x_tail(Vertex *t, ulong want)",
	"{	int i, yes, no; Edge *e; Vertex *v = (Vertex *) 0;",
	"",
	"	if (!t) return v;",
	"",
	"	yes = no = 0;",
	"	for (i = 0; i < 2; i++)",
	"		if ((ulong) t->dst[i] == want)",
	"		{	/* was t->from[i] <= 0 && t->to[i] >= 0 */",
	"			/* but from and to are uchar */",
	"			if (t->from[i] == 0)",
	"				yes = 1;",
	"			else",
	"			if (t->from[i] <= 4 && t->to[i] >= 4)",
	"				no = 1;",
	"		}",
	"",
	"	for (e = t->Succ; e; e = e->Nxt)",
	"		if ((ulong) e->Dst == want)",
	"		{	/* was INRANGE(e,0) but From and To are uchar */",
	"			if ((e->From == 0) || (e->s==1 && e->S==0))",
	"				yes = 1;",
	"			else if (INRANGE(e, 4))",
	"				no = 1;",
	"		}",
	"	if (yes && !no) return t;",
	"	v = x_tail(t->left, want);  if (v) return v;",
	"	v = x_tail(t->right, want); if (v) return v;",
	"	return (Vertex *) 0;",
	"}",
	"",
	"static void",
	"x_anytail(Vertex *t, Vertex *c, int nr)",
	"{	int i; Edge *e, *f; Vertex *v;",
	"",
	"	if (!t) return;",
	"",
	"	for (i = 0; i < 2; i++)",
	"		if ((ulong) t->dst[i] == c->key)",
	"		{	v = new_vertex(); v->key = t->key;",
	"			f = new_edge(v);",
	"			f->From = t->from[i];",
	"			f->To = t->to[i];",
	"			f->Nxt = c->Succ;",
	"			c->Succ = f;",
	"			if (nr > 0)",
	"			x_anytail(temptree[nr-1], v, nr-1);",
	"		}",
	"",
	"	for (e = t->Succ; e; e = e->Nxt)",
	"		if ((ulong) e->Dst == c->key)",
	"		{	v = new_vertex(); v->key = t->key;",
	"			f = new_edge(v);",
	"			f->From = e->From;",
	"			f->To = e->To;",
	"			f->s = e->s;",
	"			f->S = e->S;",
	"			f->Nxt = c->Succ;",
	"			c->Succ = f;",
	"			x_anytail(temptree[nr-1], v, nr-1);",
	"		}",
	"",
	"	x_anytail(t->left, c, nr);",
	"	x_anytail(t->right, c, nr);",
	"}",
	"",
	"static Vertex *",
	"x_cpy_rev(void)",
	"{	Vertex *c, *v;	/* find 0 and !4 predecessor of F */",
	"",
	"	v = x_tail(temptree[dfa_depth-1], F->key);",
	"	if (!v) return (Vertex *) 0;",
	"",
	"	c = new_vertex(); c->key = v->key;",
	"",
	"	/* every node on dfa_depth-2 that has v->key as succ */",
	"	/* make copy and let c point to these (reversing ptrs) */",
	"",
	"	x_anytail(temptree[dfa_depth-2], c, dfa_depth-2);",
	" ",
	"	return c;",
	"}",
	"",
	"void",
	"r_xpoint(void)",
	"{	int fd; char nm[64]; Vertex *d;",
	"	int i, j;",
	"",
	"	wcnt = 0;",
	"	sprintf(nm, \"%%s.xpt\", Source);",
	"	if ((fd = open(nm, 0)) < 0)	/* O_RDONLY */",
	"		Uerror(\"cannot open checkpoint file\");",
	"",
	"	xread(fd, (char *) &nstates,   sizeof(double));",
	"	xread(fd, (char *) &truncs,    sizeof(double));",
	"	xread(fd, (char *) &truncs2,   sizeof(double));",
	"	xread(fd, (char *) &nlinks,    sizeof(double));",
	"	xread(fd, (char *) &dfa_depth, sizeof(int));",
	"",
	"	if (dfa_depth != MA+a_cycles)",
	"		Uerror(\"bad dfa_depth in checkpoint file\");",
	"",
	"	path	  = (Vertex **) emalloc((dfa_depth+1)*sizeof(Vertex *));",
	"	layers	  = (Vertex **) emalloc(TWIDTH*(dfa_depth+1)*sizeof(Vertex *));",
	"	temptree  = (Vertex **) emalloc((dfa_depth+2)*sizeof(Vertex *));",
	"	lastword  = (uchar *)   emalloc((dfa_depth+1)*sizeof(uchar));",
	"	lastword[dfa_depth] = lastword[0] = 255; ",
	"",
	"	path[0] = R = new_vertex();",
	"	xread(fd, (char *) &R->key, sizeof(Vertex *));",
	"	R = insert_withkey(R, 0);",
	"",
	"	F = new_vertex();",
	"	xread(fd, (char *) &F->key, sizeof(Vertex *));",
	"	F = insert_withkey(F, dfa_depth);",
	"",
	"	NF = new_vertex();",
	"	xread(fd, (char *) &NF->key, sizeof(Vertex *));",
	"	NF = insert_withkey(NF, dfa_depth);",
	"",
	"	for (j = 0; j < TWIDTH; j++)",
	"	for (i = 0; i < dfa_depth+1; i++)",
	"		r_layer(fd, i);",
	"",
	"	if (wcnt != 0) Uerror(\"bad count in checkpoint file\");",
	"",
	"	d = x_cpy_rev();",
	"	x_fixup();",
	"	stacker[dfa_depth-1] = 0;",
	"	x_rm_stack(d, dfa_depth-2);",
	"	x_cleanup(d);",
	"	close(fd);",
	"",
	"	printf(\"pan: removed %%d stackstates\\n\", stackcnt);",
	"	nstates -= (double) stackcnt;",
	"}",
	"#endif",
	0,
};
