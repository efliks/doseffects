#ifndef _PCX_H
#define _PCX_H

typedef struct
{
    unsigned int width;
    unsigned int height;
    unsigned char* data;
    unsigned char palette[768];
} PCX;

int load_pcx(PCX* pcx, char* filename, int load_palette);
void unload_pcx(PCX* pcx);

int save_pcx(char* FileName, unsigned char* Buffer, unsigned char* Palette, 
    unsigned int Width, unsigned int Height);

#endif // _PCX_H
