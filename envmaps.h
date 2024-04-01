#ifndef _ENVMAPS_H
#define _ENVMAPS_H

int enable_envmap(void);
void disable_envmap(void);

int enable_bumpmap(void);
void disable_bumpmap(void);

extern unsigned char *envmap, *bumpmap;

#endif // _ENVMAPS_H
