/* $XConsortium: Xtrans.h,v 1.28 94/10/18 15:57:42 mor Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/* Copyright (c) 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _XTRANS_H_
#define _XTRANS_H_

#include <X11/Xfuncproto.h>
#include <X11/Xos.h>


/*
 * Set the functions names according to where this code is being compiled.
 */

#ifdef X11_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _X11Trans##func
#else
#define TRANS(func) _X11Trans/**/func
#endif
#endif /* X11_t */

#ifdef XSERV_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XSERVTrans##func
#else
#define TRANS(func) _XSERVTrans/**/func
#endif
#define X11_t
#endif /* X11_t */

#ifdef XIM_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XimdXTrans##func
#else
#define TRANS(func) _XimdXTrans/**/func
#endif
#endif /* XIM_t */

#ifdef FS_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FSTrans##func
#else
#define TRANS(func) _FSTrans/**/func
#endif
#endif /* FS_t */

#ifdef FONT_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FontTrans##func
#else
#define TRANS(func) _FontTrans/**/func
#endif
#endif /* FONT_t */

#ifdef ICE_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _IceTrans##func
#else
#define TRANS(func) _IceTrans/**/func
#endif
#endif /* ICE_t */

#ifdef TEST_t
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _TESTTrans##func
#else
#define TRANS(func) _TESTTrans/**/func
#endif
#endif /* TEST_t */

#if !defined(TRANS)
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XTrans##func
#else
#define TRANS(func) _XTrans/**/func
#endif
#endif /* !TRANS */


/*
 * Create a single address structure that can be used wherever
 * an address structure is needed. struct sockaddr is not big enough
 * to hold a sockadd_un, so we create this definition to have a single
 * structure that is big enough for all the structures we might need.
 *
 * This structure needs to be independent of the socket/TLI interface used.
 */

#define XTRANS_MAX_ADDR_LEN	128	/* large enough to hold sun_path */

typedef	struct {
    unsigned char	addr[XTRANS_MAX_ADDR_LEN];
} Xtransaddr;


#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif


#if defined(WIN32) || (defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu) && !defined(__sxg__))

/*
 *      TRANS(Readv) and TRANS(Writev) use struct iovec, normally found
 *      in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *      manual pages for details.
 */

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

#else
#include <sys/uio.h>
#endif

typedef struct _XtransConnInfo *XtransConnInfo;


/*
 * Transport Option definitions
 */

#define TRANS_NONBLOCKING	1
#define	TRANS_CLOSEONEXEC	2


/*
 * Return values of Connect (0 is success)
 */

#define TRANS_CONNECT_FAILED 	-1
#define TRANS_TRY_CONNECT_AGAIN -2


/*
 * Return values of CreateListener (0 is success)
 */

#define TRANS_CREATE_LISTENER_FAILED 	-1
#define TRANS_ADDR_IN_USE		-2


/*
 * Return values of Accept (0 is success)
 */

#define TRANS_ACCEPT_BAD_MALLOC			-1
#define TRANS_ACCEPT_FAILED 			-2
#define TRANS_ACCEPT_MISC_ERROR			-3


/*
 * ResetListener return values
 */

#define TRANS_RESET_NOOP	1
#define TRANS_RESET_NEW_FD	2
#define TRANS_RESET_FAILURE	3


/*
 * Function prototypes for the exposed interface
 */

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCOTSClient)(
    char *		/* address */
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCOTSServer)(
    char *		/* address */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCLTSClient)(
    char *		/* address */
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCLTSServer)(
    char *		/* address */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_REOPEN

XtransConnInfo TRANS(ReopenCOTSServer)(
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
);

XtransConnInfo TRANS(ReopenCLTSServer)(
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
);

int TRANS(GetReopenInfo)(
    XtransConnInfo,	/* ciptr */
    int *,		/* trans_id */
    int *,		/* fd */
    char **		/* port */
);

#endif /* TRANS_REOPEN */


int TRANS(SetOption)(
    XtransConnInfo,	/* ciptr */
    int,		/* option */
    int			/* arg */
);

#ifdef TRANS_SERVER

int TRANS(CreateListener)(
    XtransConnInfo,	/* ciptr */
    char *		/* port */
);

int TRANS(ResetListener)(
    XtransConnInfo	/* ciptr */
);

XtransConnInfo TRANS(Accept)(
    XtransConnInfo,	/* ciptr */
    int *		/* status */
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

int TRANS(Connect)(
    XtransConnInfo,	/* ciptr */
    char *		/* address */
);

#endif /* TRANS_CLIENT */

int TRANS(BytesReadable)(
    XtransConnInfo,	/* ciptr */
    BytesReadable_t *	/* pend */
);

int TRANS(Read)(
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
);

int TRANS(Write)(
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
);

int TRANS(Readv)(
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
);

int TRANS(Writev)(
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
);

int TRANS(Disconnect)(
    XtransConnInfo	/* ciptr */
);

int TRANS(Close)(
    XtransConnInfo	/* ciptr */
);

int TRANS(CloseForCloning)(
    XtransConnInfo	/* ciptr */
);

int TRANS(IsLocal)(
    XtransConnInfo	/* ciptr */
);

int TRANS(GetMyAddr)(
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

int TRANS(GetPeerAddr)(
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

int TRANS(GetConnectionNumber)(
    XtransConnInfo	/* ciptr */
);

#ifdef TRANS_SERVER

int TRANS(MakeAllCOTSServerListeners)(
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
);

int TRANS(MakeAllCLTSServerListeners)(
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
);

#endif /* TRANS_SERVER */


/*
 * Function Prototypes for Utility Functions.
 */

#ifdef X11_t

int TRANS(ConvertAddress)(
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
);

#endif /* X11_t */

#ifdef ICE_t

char *
TRANS(GetMyNetworkId)(
    XtransConnInfo	/* ciptr */
);

char *
TRANS(GetPeerNetworkId)(
    XtransConnInfo	/* ciptr */
);

#endif /* ICE_t */

#if defined(WIN32) && (defined(TCPCONN) || defined(DNETCONN))
int TRANS(WSAStartup)();
#endif

#endif /* _XTRANS_H_ */
