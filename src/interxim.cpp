/******************************************************************
         Copyright (c) 2002 by O'ksi'D
         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company
 
Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
 
SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.
 
******************************************************************/
#include <stdio.h>
#include <locale.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <IMdkit.h>
#include <Xi18n.h>
#include "IC.h"
#include "interface.h"
#include <libXutf8/Xutf8.h>

#define DEFAULT_IMNAME "interxim"
#define DEFAULT_LOCALE ",C,en_US,ja_JP,zh_CN,zh_HK,zh_TW,ko_KR"
#define IC_SHOW 1

/* flags for debugging */
Bool use_trigger = True;	/* Dynamic Event Flow is default */
Bool use_offkey = False;	/* Register OFF Key for Dynamic Event Flow */
Bool use_tcp = False;		/* Using TCP/IP Transport or not */
Bool use_local = False;		/* Using Unix domain Tranport or not */
long filter_mask = KeyPressMask|KeyReleaseMask;
Window im_window;
Display *dpy;
char is_dead_key = 0;
XIMS dead_ims = NULL;
IMCommitStruct dead_c;
extern XIC fl_xim_ic;
XIM global_im = NULL;
XIC global_ic = NULL;

/* Supported Inputstyles */
static XIMStyle Styles[] = {
/*    XIMPreeditCallbacks|XIMStatusCallbacks,
    XIMPreeditPosition|XIMStatusArea,
    XIMPreeditPosition|XIMStatusNothing,
    XIMPreeditArea|XIMStatusArea,*/
    XIMPreeditNothing|XIMStatusNothing,
    0
};

/* Trigger Keys List */
static XIMTriggerKey Trigger_Keys[] = {
    {XK_space, ShiftMask, ShiftMask},
    {XK_dead_grave, 0, 0},
    {XK_dead_acute, 0, 0},
    {XK_dead_circumflex, 0, 0},
    {XK_dead_tilde, 0L, 0L},
    {XK_dead_macron, 0L, 0L},
    {XK_dead_breve, 0L, 0L},
    {XK_dead_abovedot, 0L, 0L},
    {XK_dead_diaeresis, 0L, 0L},
    {XK_dead_abovering, 0L, 0L},
    {XK_dead_doubleacute, 0L, 0L},
    {XK_dead_caron, 0L, 0L},
    {XK_dead_cedilla, 0L, 0L},
    {XK_dead_ogonek, 0L, 0L},
    {XK_dead_iota, 0L, 0L},
    {XK_dead_voiced_sound, 0L, 0L},
    {XK_dead_semivoiced_sound, 0L, 0L},
    {0xFE61 /*XK_dead_hook*/, 0L, 0L},
    {0xFE62 /*XK_dead_horn*/, 0L, 0L},
    {0L, 0L, 0L}
};

/* Real Trigger Keys List */
static XIMTriggerKey RealTrigger_Keys[] = {
    {XK_space, ShiftMask, ShiftMask},
    {0L, 0L, 0L}
};

/* Conversion Keys List */
static XIMTriggerKey Conversion_Keys[] = {
    {XK_k, ControlMask, ControlMask},
    {XK_Return, 0, 0},
    {0L, 0L, 0L}
};

/* Forward Keys List */
static XIMTriggerKey Forward_Keys[] = {
    {XK_Tab, 0, 0},
    {0L, 0L, 0L}
};

/* Supported Taiwanese Encodings */
static XIMEncoding zhEncodings[] = {
    "STRING",
    /*"COMPOUND_TEXT",*/
    NULL
};

int
IxmGetICValuesHandler(XIMS ims, IMChangeICStruct *call_data)
{
    GetIC(call_data);
    return True;
}

int
IxmSetICValuesHandler(XIMS ims, IMChangeICStruct *call_data)
{
    SetIC(call_data);
    return True;
}

int
IxmOpenHandler(XIMS ims, IMOpenStruct *call_data)
{
#ifdef DEBUG
    printf("new_client lang = %s\n", call_data->lang.name);
    printf("     connect_id = 0x%x\n", (int)call_data->connect_id);
#endif
    return True;
}

int
IxmCloseHandler(XIMS ims, IMOpenStruct *call_data)
{
#ifdef DEBUG
    printf("closing connect_id 0x%x\n", (int)call_data->connect_id);
#endif
    return True;
}

int
IxmCreateICHandler(XIMS ims, IMChangeICStruct *call_data)
{
    CreateIC(call_data);
    return True;
}

int
IxmDestroyICHandler(XIMS ims, IMChangeICStruct *call_data)
{
    DestroyIC(call_data);
    return True;
}

#define STRBUFLEN 64
int
IsKey(XIMS ims, IMForwardEventStruct *call_data, XIMTriggerKey *trigger)
{
    char strbuf[STRBUFLEN];
    KeySym keysym;
    int i;
    int modifier;
    int modifier_mask;
    XKeyEvent *kev;

    memset(strbuf, 0, STRBUFLEN);
    kev = (XKeyEvent*)&call_data->event;
    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);

    for (i = 0; trigger[i].keysym != 0; i++) {
	modifier      = trigger[i].modifier;
	modifier_mask = trigger[i].modifier_mask;
	if (((KeySym)trigger[i].keysym == keysym)
	    && ((kev->state & modifier_mask) == (unsigned int)modifier))
	  return True;
    }
    return False;
}

void
ProcessKey(XIMS ims, IMForwardEventStruct *call_data)
{
    //static Status status;
    char strbuf[STRBUFLEN];
    KeySym keysym;
    XKeyEvent *kev;
    int count;
   // int filtered = -1;
   // fprintf(stderr, "Processing \n");
    memset(strbuf, 0, STRBUFLEN);
    kev = (XKeyEvent*)&call_data->event;
    kev->window = im_window;
    //filtered = XFilterEvent((XEvent *)&kev, 0);
    //if (filtered > 0 && fl_xim_ic) return;
    
   // if (0 && fl_xim_ic) {
//	count = XUtf8LookupString(fl_xim_ic, (XKeyPressedEvent *)kev,
//		    	strbuf, STRBUFLEN, &keysym, &status);	
  //  } else {
    	count = XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);
   // }
    IxmHandleKex(keysym, strbuf, count);
}

XKeyEvent *
IxmSendEvent(unsigned int keycode, unsigned int state)
{
	static XKeyEvent ev;
	ev.type = KeyPress;
	ev.serial = 0;
        ev.send_event = True;
	ev.display = dpy;
	ev.window = im_window;
	ev.root =  DefaultRootWindow (dpy);
	ev.subwindow = 0;
	ev.time = 0;
	ev.x = 0;
	ev.y = 0;
	ev.x_root = 0;
	ev.y_root = 0;
        ev.state = state;
	ev.keycode = keycode;
	ev.same_screen = True;
	//XSendEvent(dpy, im_window, True, KeyPressMask,(XEvent*) &ev);
	XFilterEvent((XEvent*)&ev, None);
	return &ev;
}

int
IxmForwardEventHandler(XIMS ims, IMForwardEventStruct *call_data)
{
    /* Lookup KeyPress Events only */
    //fprintf(stderr, "ForwardEventHandler\n");
    XKeyEvent *kev;
    if (call_data->event.type != KeyPress) {
       // fprintf(stderr, "bogus event type, ignored\n");
    	return True;
    }
    kev = (XKeyEvent*)&call_data->event;

    /* commit dead_key/letter sequence */
    if (is_dead_key == 1) {
	char b[20];
	KeySym keysym;
	static Status s;
	int len;
	IMCommitStruct *c = (IMCommitStruct*)call_data;
	kev = IxmSendEvent(kev->keycode, kev->state);
	dead_ims = ims;
	dead_c.connect_id = c->connect_id;
	dead_c.flag = c->flag;
	dead_c.icid = c->icid;
	if (global_ic) {
	  len = XmbLookupString(global_ic, (XKeyPressedEvent*)kev, 
		b, 20, &keysym, &s);
	} else {
		static XComposeStatus s;
		len = XLookupString((XKeyPressedEvent*)kev,
			b, 20, &keysym, &s);
	}
	if (len < 0) len = 0;
	b[len] = 0;
	dead_c.flag = XimLookupKeySym;
	dead_c.commit_string = b;
	dead_c.keysym = keysym;
	IMCommitString(dead_ims, (XPointer)&dead_c);
	is_dead_key = 0;
	IMPreeditEnd(dead_ims, (XPointer)&dead_c);
	return True;
    } else if (is_dead_key > 0) {
	return False;
    }

    if (kev->keycode == 0) {
	/* this event is generated by ourself */
        IMForwardEventStruct forward_ev = *((IMForwardEventStruct *)call_data);
	IMForwardEvent(ims, (XPointer)&forward_ev);
	return True;
    }
    if (IsKey(ims, call_data, Conversion_Keys) || 
	IsKey(ims, call_data, RealTrigger_Keys)) 
    {
	unsigned short *ret;
	int l, i;
	l = IxmGetUnicodeBuffer(&ret);
	//fprintf(stderr, "matching ctrl-k...\n");
	i = 0;
	while (i < l) {
		((IMCommitStruct*)call_data)->flag |= XimLookupKeySym; 
		((IMCommitStruct*)call_data)->commit_string = "";
		((IMCommitStruct*)call_data)->keysym = 0x01000000 + ret[i];
		IMCommitString(ims, (XPointer)call_data);
		i++;
	}
	if (IsKey(ims, call_data, RealTrigger_Keys)) {
		IC *ic = FindIC(call_data->icid);
		if (ic) ic->state &= ~IC_SHOW;
		IxmHide();
		return IMPreeditEnd(ims, (XPointer)call_data);
	}
    }
    else if (IsKey(ims, call_data, Forward_Keys)) {
        IMForwardEventStruct forward_ev = *((IMForwardEventStruct *)call_data);
	//fprintf(stderr, "TAB and RETURN forwarded...\n");
	IMForwardEvent(ims, (XPointer)&forward_ev);
    } else {
	ProcessKey(ims, call_data);
    }
    return True;
}

unsigned int
IxmGetKeyState(KeySym k)
{
	KeySym key = 0;
	char b[20];
	static XKeyEvent ev;
	ev.type = KeyPress;
	ev.serial = 0;
        ev.send_event = True;
	ev.display = dpy;
	ev.window = im_window;
	ev.root =  DefaultRootWindow (dpy);
	ev.subwindow = 0;
	ev.time = 0;
	ev.x = 0;
	ev.y = 0;
	ev.x_root = 0;
	ev.y_root = 0;
        ev.state = 0;
	ev.keycode = XKeysymToKeycode(dpy, k);
	ev.same_screen = True;
	
	for (int i = 0; i < 16; i++) {
		XComposeStatus o;
		ev.state = 1 << i;
		XLookupString(&ev, b, 20, &key, &o);
		if (key ==k) {
			return ev.state;
		}
	}
	return 0;
}

int 
IxmTriggerNotifyHandler(XIMS ims, IMTriggerNotifyStruct *call_data)
{
    if (call_data->flag == 0) {	/* on key */
	//printf("on key %d\n", call_data->key_index);
	if (call_data->key_index > 0 && is_dead_key == 0) {
		KeySym k = Trigger_Keys[call_data->key_index / 3].keysym;
		unsigned int s = IxmGetKeyState(k);
		IxmSendEvent(XKeysymToKeycode(dpy, k), s);
		is_dead_key = 1;
	} else {
		IC *ic = FindIC(call_data->icid);
		if (ic) ic->state |= IC_SHOW;
		IxmShow();
		is_dead_key = 0;
	}
	/* Here, the start of preediting is notified from IMlibrary, which 
	   is the only way to start preediting in case of Dynamic Event
	   Flow, because ON key is mandatary for Dynamic Event Flow. */
	 //IMPreeditStart(ims, (XPointer)call_data);
	return True;
    } else if (use_offkey && call_data->flag == 1) {	/* off key */
	/* Here, the end of preediting is notified from the IMlibrary, which
	   happens only if OFF key, which is optional for Dynamic Event Flow,
	   has been registered by IMOpenIM or IMSetIMValues, otherwise,
	   the end of preediting must be notified from the IMserver to the
	   IMlibrary. */
	is_dead_key = 0;

	return True;
    } else {
	/* never happens */
	return False;
    }
}

int
IxmPreeditStartReplyHandler(XIMS ims, IMPreeditCBStruct *call_data)
{
	return False;
}

int
IxmPreeditCaretReplyHandler(XIMS ims, IMPreeditCBStruct *call_data)
{
	return False;
}

int
IxmProtoHandler(XIMS ims, IMProtocol *call_data)
{
   //fprintf(stderr, "IxmProtoHandler: %d\n", call_data->major_code);
    switch (call_data->major_code) {
      case XIM_OPEN:
       // fprintf(stderr, "XIM_OPEN:\n");
	return IxmOpenHandler(ims, (IMOpenStruct*)call_data);
      case XIM_CLOSE:
        //fprintf(stderr, "XIM_CLOSE:\n");
	return IxmCloseHandler(ims, (IMOpenStruct*)call_data);
      case XIM_CREATE_IC:
        //fprintf(stderr, "XIM_CREATE_IC:\n");
	return IxmCreateICHandler(ims, (IMChangeICStruct*)call_data);
      case XIM_DESTROY_IC:
        //fprintf(stderr, "XIM_DESTROY_IC.\n");
        return IxmDestroyICHandler(ims, (IMChangeICStruct*)call_data);
      case XIM_SET_IC_VALUES:
        //fprintf(stderr, "XIM_SET_IC_VALUES:\n");
	return IxmSetICValuesHandler(ims, (IMChangeICStruct*)call_data);
      case XIM_GET_IC_VALUES:
        //fprintf(stderr, "XIM_GET_IC_VALUES:\n");
	return IxmGetICValuesHandler(ims, (IMChangeICStruct*)call_data);
      case XIM_FORWARD_EVENT:
	return IxmForwardEventHandler(ims, (IMForwardEventStruct*)call_data);
      case XIM_SET_IC_FOCUS: {
        //fprintf(stderr, "XIM_SET_IC_FOCUS()\n");
	IC *ic = FindIC(((IMChangeFocusStruct*)call_data)->icid);
	if (ic != NULL  && (ic->state & IC_SHOW)) {
		IxmShow();
	}
	return True;
      }
      case XIM_UNSET_IC_FOCUS:
        //fprintf(stderr, "XIM_UNSET_IC_FOCUS:\n");
	IxmLower();
	return True;
      case XIM_RESET_IC:
        //fprintf(stderr, "XIM_RESET_IC_FOCUS:\n");
	return True;
      case XIM_TRIGGER_NOTIFY:
        //fprintf(stderr, "XIM_TRIGGER_NOTIFY:\n");
	return IxmTriggerNotifyHandler(ims, 
		(IMTriggerNotifyStruct*)call_data);
      case XIM_PREEDIT_START_REPLY:
        //fprintf(stderr, "XIM_PREEDIT_START_REPLY:\n");
	return IxmPreeditStartReplyHandler(ims, 
		(IMPreeditCBStruct*)call_data);
      case XIM_PREEDIT_CARET_REPLY:
        //fprintf(stderr, "XIM_PREEDIT_CARET_REPLY:\n");
	return IxmPreeditCaretReplyHandler(ims, 
		(IMPreeditCBStruct*)call_data);
      default:
	//fprintf(stderr, "Unknown IMDKit Protocol message type\n");
	break;
    }
    return 0;
}

int
main(int argc, char **argv)
{
    char *display_name = NULL;
    char *imname = NULL;
    XIMS ims;
    XIMStyles *input_styles, *styles2;
    XIMTriggerKeys *on_keys, *trigger2;
    XIMEncodings *encodings, *encoding2;
    register int i;
    char transport[80];		/* enough */
    char *loc;

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-name")) {
	    imname = argv[++i];
	} else if (!strcmp(argv[i], "-display")) {
	    display_name = argv[++i];
	}
    }
    if (!imname) imname = DEFAULT_IMNAME;

    setlocale(LC_CTYPE, "");
    XSetLocaleModifiers("@im=");
    if ((dpy = IxmOpenDisplay(display_name)) == NULL) {
	fprintf(stderr, "Can't Open Display: %s\n", display_name);
	exit(1);
    }
    im_window = IxmCreateSimpleWindow(dpy);

    if (im_window == (Window)NULL) {
	fprintf(stderr, "Can't Create Window\n");
	exit(1);
    }
    XStoreName(dpy, im_window, "interxim");
    XSetTransientForHint(dpy, im_window, im_window);
    global_im = XOpenIM(dpy, NULL, "interxim", "Interxim");
    if (global_im) {
	global_ic = XCreateIC(global_im,
		XNInputStyle, (XIMPreeditNothing | XIMStatusNothing),
		XNClientWindow, im_window,
		XNFocusWindow, im_window, NULL);
	XSetICFocus(global_ic);
    } else {
	global_ic = NULL;
    }
    if ((input_styles = (XIMStyles *)malloc(sizeof(XIMStyles))) == NULL) {
	fprintf(stderr, "Can't allocate\n");
	exit(1);
    }
    input_styles->count_styles = sizeof(Styles)/sizeof(XIMStyle) - 1;
    input_styles->supported_styles = Styles;

    if ((on_keys = (XIMTriggerKeys *)
	 malloc(sizeof(XIMTriggerKeys))) == NULL) {
	fprintf(stderr, "Can't allocate\n");
	exit(1);
    }
    on_keys->count_keys = sizeof(Trigger_Keys)/sizeof(XIMTriggerKey) - 1;
    on_keys->keylist = Trigger_Keys;

    if ((encodings = (XIMEncodings *)malloc(sizeof(XIMEncodings))) == NULL) {
	fprintf(stderr, "Can't allocate\n");
	exit(1);
    }
    encodings->count_encodings = sizeof(zhEncodings)/sizeof(XIMEncoding) - 1;
    encodings->supported_encodings = zhEncodings;

    strcpy(transport, "X/");
    loc = (char*) malloc(strlen(DEFAULT_LOCALE) + 
		strlen(setlocale(LC_CTYPE, NULL)) + 1);
    strcpy(loc, setlocale(LC_CTYPE, NULL));
    strcpy(loc + strlen(loc), DEFAULT_LOCALE);
    ims = IMOpenIM(dpy,
		   IMModifiers, "Xi18n",
		   IMServerWindow, im_window,
		   IMServerName, imname,
		   IMLocale, loc, 
		   IMServerTransport, transport,
		   IMInputStyles, input_styles,
		   NULL);
    if (ims == (XIMS)NULL) {
	fprintf(stderr, "Can't Open Input Method Service:\n");
	fprintf(stderr, "\tInput Method Name :%s\n", imname);
	fprintf(stderr, "\tTranport Address:%s\n", transport);
	exit(1);
    }
    IMSetIMValues(ims, IMOnKeysList, on_keys, NULL);
    IMSetIMValues(ims,
		  IMEncodingList, encodings,
		  IMProtocolHandler, IxmProtoHandler,
		  IMFilterEventMask, filter_mask,
		  NULL);
    IMGetIMValues(ims,
		  IMInputStyles, &styles2,
		  IMOnKeysList, &trigger2,
		  IMOffKeysList, &trigger2,
		  IMEncodingList, &encoding2,
		  NULL);
    //XSelectInput(dpy, im_window, StructureNotifyMask|ButtonPressMask);
    /*XMapWindow(dpy, im_window);*/
    XFlush(dpy);		/* necessary flush for tcp/ip connection */
    return IxmMainLoop(dpy, im_window);
}
