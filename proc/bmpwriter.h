#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


template <typename Iterator>
void fwritebmp(FILE *out, uint16_t width, uint16_t height, Iterator first, Iterator last) {
    size_t ndata = last - first;
    const unsigned bits_per_pixel = ndata * 8 / width / height;
#define write2(x) ({uint16_t tmp = x; fwrite(&tmp, 1, sizeof(tmp), out);})
#define write4(x) ({uint32_t tmp = x; fwrite(&tmp, 1, sizeof(tmp), out);})
    /*=======================================================================*/
    /*            write bmp file according to the bmp standard               */
    /*=======================================================================*/
    /* offset */                              /* field description           */
    /*   0    */ fputs("BM", out);            /* signature                   */
    /*   2    */ write4(ndata + 26);          /* file size                   */
    /*   6    */ write4(0);                   /* reserved                    */
    /*   10   */ write4(26);                  /* pixels data offset          */
    /*   14   */ write4(12);                  /* following header size       */
    /*   18   */ write2(width);               /* width                       */
    /*   20   */ write2(height);              /* height                      */
    /*   22   */ write2(1);                   /* number of planes (reserved) */
    /*   24   */ write2(bits_per_pixel);      /* bits per pixel              */
    /*   26   */ for (Iterator it = first; it != last; ++it)                 // 
    /*        */     fputc((unsigned char)*it, out);                 /* data */
    /*=======================================================================*/
#undef write2
#undef write4
}


void fwritebmp(FILE* out, uint16_t width, uint16_t height, size_t ndata, const char* data) {
    fwritebmp(out, width, height, data, data + ndata);
}
