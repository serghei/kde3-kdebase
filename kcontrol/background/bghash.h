#ifndef BGHASH_H
#define BGHASH_H

/*
 * QString -> int hash. From Qt's QGDict::hashKeyString().
 */

static int QHash(QString key)
{
    int g, h = 0;
    const QChar *p = key.unicode();
    for (unsigned i=0; i < key.length(); i++) {
        h = (h << 4) + p[i].cell();
        if ((g = (h & 0xf0000000)))
            h ^= (g >> 24);
        h &= ~g;
    }
    return h;
}

#endif

