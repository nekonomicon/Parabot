/* nwreckdum Quake .pak manipulator for linux by Daniel Reed <n@ml.org>
based on wreckdum by Neal Tucker <ntucker@fester.axis.net>
*/
#if defined(__ANDROID__)
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define kBufferSize 10000
typedef struct {
	char	name[56];
	uint32_t	offset, length;
} dirrec;

static int	makepath(char *fname) {
/* given "some/path/to/file", makes all dirs (ie "some/", "some/path/", */
/* and "some/path/to/", so that file can be created                     */
	char	*c = fname,
		*o = (char *)malloc(strlen(fname)+1),
		*n = o;

	while(*c) {
		while(*c && (*c != '/'))
			*n++ = *c++;
		if(*c) {
			*n = '\0';
			mkdir(o, 493 /*octal=755*/);
			*n = '/';
			n++;
			c++;
		}
	}
	free(o);
	return(0);
}

static int	write_rec(dirrec dir, FILE *in) {
	char	buf[kBufferSize];
	FILE	*f = NULL;
	long	l = 0;
	int	blocks = 0, count = 0, s = 0;

	makepath(dir.name);
	if ((f = fopen(dir.name,"wb")) == NULL) {
		return(-1);
	}
	fseek(in, dir.offset, SEEK_SET);
	blocks = dir.length / (kBufferSize-1);
	if (dir.length%(kBufferSize-1))
		blocks++;
	for (count = 0; count < blocks; count++) {
		fread((void *)buf, 1, (kBufferSize-1), in);
		if ((l = dir.length - (long)count * (kBufferSize-1)) >
			(kBufferSize-1))
			l = (kBufferSize-1);
		s = l;
		fwrite((void *)buf, 1, s, f);
	}
	fclose(f);
	return(0);
}

static int	extrpak(FILE *in, char *mdir) {
	dirrec	*dir = NULL;
	char	signature[4];
	unsigned long	dir_start = 0, num_ents = 0;
	int	i = 0;

	fread((void *)signature, 4, 1, in);
	if (strncmp(signature, "PACK", 4) != 0) {
		fclose(in);
		return(-1);
	}

	fread((void *)&dir_start, 4, 1, in);
	fread((void *)&num_ents, 4, 1, in);
	num_ents /= sizeof(dirrec);
	fseek(in, dir_start, SEEK_SET);

	if ((dir = (dirrec*)calloc(num_ents, sizeof(dirrec))) == NULL) {
		fclose(in);
		return(-1);
	}

	fread((void *)dir, sizeof(dirrec), num_ents, in);

	for (i = 0; i < num_ents; i++) {
	char pdir[100];
	sprintf(pdir,"%s/%s",mdir,dir[i].name);
	strcpy(dir[i].name, pdir);
		if (write_rec(dir[i], in))
			break;
	}
	free(dir);
	return(0);
}
#endif
