/*
 * Copyright (c) 1995,1999 Theo de Raadt.  All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#include "includes.h"
// RCSID("$OpenBSD: atomicio.c,v 1.9 2001/03/02 18:54:30 deraadt Exp $");

//#include "xmalloc.h"
#include "atomicio.h"
#include <unistd.h>
#include <errno.h>
#include <kdebug.h>

/*
 * ensure all of data on socket comes through. f==read || f==write
 */

ssize_t atomicio(int fd, char *_s, size_t n, bool read)
{
    char *s = _s;
    ssize_t res;
    ssize_t pos = 0;

    while(n > pos)
    {
        if(read)
            res = ::read(fd, s + pos, n - pos);
        else
            res = ::write(fd, s + pos, n - pos);

        switch(res)
        {
            case -1:
                kdDebug() << "atomicio(): errno=" << errno << endl;
#ifdef EWOULDBLOCK
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
#else
                if(errno == EINTR || errno == EAGAIN)
#endif
                    continue;
            case 0:
                return (res);
            default:
                pos += res;
        }
    }
    return (pos);
}
