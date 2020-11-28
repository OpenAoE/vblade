/* dat.c: Global memory for vblade AoE target */
#include "dat.h"

int	shelf, slot;
ulong	aoetag;
uchar	mac[6];
int	bfd;		// block file descriptor
int	sfd;		// socket file descriptor
vlong	size;		// size of vblade
vlong	offset;
char	*progname;
char	serial[Nserial+1];
