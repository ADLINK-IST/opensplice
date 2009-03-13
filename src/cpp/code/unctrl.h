#ifndef unctrl
#define unctrl(c) _unctrl[0xff&(int)(c)]
#define Unctrl(c) _Unctrl[0xff&(int)(c)]
extern char *_unctrl[];
extern char *_Unctrl[];
#endif
