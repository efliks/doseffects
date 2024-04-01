/*
	PCX
	Mikolaj Felix 13/07/01
	mfelix@polbox.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcx.h"

char error_message[128];

typedef struct tPCXHEAD 
{
    unsigned char Manufacter; // 0x0a
    unsigned char Version;
    unsigned char Encoding; // 0x01
    unsigned char BitsPerPixel;

    unsigned short XMin;
    unsigned short YMin;
    unsigned short XMax;
    unsigned short YMax;
    unsigned short HRes;
    unsigned short VRes;

    unsigned char Palette[48];
    unsigned char Reserved;
    unsigned char NumOfPlanes;

    unsigned short BytesPerLine;
    unsigned short PaletteType;
    unsigned short HSize;
    unsigned short VSize;

    unsigned char Filler[54];
} PCXHEAD;


int load_pcx(PCX* pcx, char* filename, int load_palette)
{
    FILE* p;
    unsigned i;
    int j;
    unsigned char b;

    if ((p = fopen(filename, "r+b")) == NULL) {
        strcpy(error_message, "Unable to open file!");
        return 0;
    }

    if (fgetc(p) != 0x0a) {
        fclose(p);
        strcpy(error_message, "Incorrect file format!");
        return 0;
    }

    fseek(p, 2, SEEK_CUR);
    if (fgetc(p) != 8) {
        fclose(p);
        strcpy(error_message, "Incorrect color depth. Must by 8-bit!");
        return 0;
    }

    fread(&i, 2, 1, p);
    pcx->width = i;
    fread(&i, 2, 1, p);
    pcx->height = i;
    fread(&i, 2, 1, p);
    pcx->width = i - pcx->width + 1;
    fread(&i, 2, 1, p);
    pcx->height = i - pcx->height + 1;

    if (pcx->width > 320 || pcx->height > 200) {
        fclose(p);
        strcpy(error_message, "Image is too big!");
        return 0;
    }

    if ((pcx->data = (unsigned char*)malloc(pcx->width * pcx->height)) == NULL) {
        fclose(p);
        strcpy(error_message, "Not enough memory!");
        return 0;
    }

    fseek(p, 128, SEEK_SET);
    i = 0;
    while (i != (pcx->width * pcx->height)) {
        b = fgetc(p);
        if (b >= 192) {
            j = b - 192;
            b = fgetc(p);
            while (j-- > 0)
                pcx->data[i++] = b;
        } else
            pcx->data[i++] = b;
    }

    if (load_palette) {
        fseek(p, -768L, SEEK_END);
        for (i = 0; i < 768; i++)
            pcx->palette[i] = fgetc(p) >> 2;
    }

    fclose(p);
    return 1;
}

void unload_pcx(PCX* pcx)
{
    free(pcx->data);
}

int save_pcx(char* FileName, unsigned char* Buffer, unsigned char* Palette, 
    unsigned int Width, unsigned int Height)
{
    PCXHEAD Head;
    FILE* FPtr;
    unsigned char c, cc, n_found;
    unsigned short i, j, FileSize, ImgSize = Width * Height;

    memset(&Head, 0, sizeof(PCXHEAD));

    Head.Manufacter = 0x0a;
    Head.Version = 5;
    Head.Encoding = 1;
    Head.BitsPerPixel = 8;
    Head.XMin = 0;
    Head.YMin = 0;
    Head.XMax = Width - 1;
    Head.YMax = Height - 1;
    Head.HRes = Width;
    Head.VRes = Height;
    Head.NumOfPlanes = 1;
    Head.BytesPerLine = Width;
    Head.PaletteType = 1;
    Head.HSize = Width;
    Head.VSize = Height;

    FPtr = fopen(FileName, "w+b");
    if (!FPtr)
        return 0;

    fwrite(&Head, sizeof(PCXHEAD), 1, FPtr);

    i = 0;
    while (i != ImgSize) {
        c = Buffer[i];

        if (c >= 192) {
            cc = 192 | 1;
            fputc(cc, FPtr);
            fputc(c, FPtr);
            i++;
            continue;
        }

        n_found = 0;
        for (j = 1; j < 63; j++) {
            if ((i + j) == ImgSize)
                break;

            if (Buffer[i + j] == c)
                n_found++;
            else
                break;
        }

        if (n_found) {
            cc = 192 | n_found;
            fputc(cc, FPtr);
            fputc(c, FPtr);
            i += n_found;
        } else {
            fputc(c, FPtr);
            i++;
        }
    }

    for (i = 0; i < 768; i++) {
        fputc(Palette[i] << 2, FPtr);
    }

    FileSize = ftell(FPtr);
    fclose(FPtr);

    return FileSize;
}
