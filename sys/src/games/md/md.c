#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <keyboard.h>
#include <mouse.h>
#include "dat.h"
#include "fns.h"

u16int *prg;
int nprg;

int keys;

int scale, paused;
QLock pauselock;
Mousectl *mc;
Rectangle picr;
Image *tmp, *bg;

void
loadrom(char *file)
{
	static uchar hdr[512], buf[4096];
	u32int v;
	u16int *p;
	int fd, rc, i;
	
	fd = open(file, OREAD);
	if(fd < 0)
		sysfatal("open: %r");
	if(readn(fd, hdr, 512) < 512)
		sysfatal("read: %r");
	if(memcmp(hdr + 0x100, "SEGA MEGA DRIVE ", 16) != 0 && memcmp(hdr + 0x100, "SEGA GENESIS    ", 16) != 0)
		sysfatal("invalid rom");
	v = hdr[0x1a0] << 24 | hdr[0x1a1] << 16 | hdr[0x1a2] << 8 | hdr[0x1a3];
	if(v != 0)
		sysfatal("rom starts at nonzero address");
	v = hdr[0x1a4] << 24 | hdr[0x1a5] << 16 | hdr[0x1a6] << 8 | hdr[0x1a7];
	nprg = v = v+2 & ~1;
	if(nprg == 0)
		sysfatal("invalid rom");
	p = prg = malloc(v);
	if(prg == nil)
		sysfatal("malloc: %r");
	seek(fd, 0, 0);
	while(v != 0){
		rc = readn(fd, buf, sizeof buf);
		if(rc == 0)
			break;
		if(rc < 0)
			sysfatal("read: %r");
		if(rc > v)
			rc = v;
		for(i = 0; i < rc; i += 2)
			*p++ = buf[i] << 8 | buf[i+1];
		v -= rc;
	}
	close(fd);
}

void
screeninit(void)
{
	Point p;

	originwindow(screen, Pt(0, 0), screen->r.min);
	p = divpt(addpt(screen->r.min, screen->r.max), 2);
	picr = (Rectangle){subpt(p, Pt(scale * 160, scale * 112)), addpt(p, Pt(scale * 160, scale * 112))};
	tmp = allocimage(display, Rect(0, 0, scale * 320, scale > 1 ? 1 : scale * 224), XRGB32, scale > 1, 0);
	bg = allocimage(display, Rect(0, 0, 1, 1), screen->chan, 1, 0xCCCCCCFF);
	draw(screen, screen->r, bg, nil, ZP);	
}

void
keyproc(void *)
{
	int fd, k;
	static char buf[256];
	char *s;
	Rune r;

	fd = open("/dev/kbd", OREAD);
	if(fd < 0)
		sysfatal("open: %r");
	for(;;){
		if(read(fd, buf, sizeof(buf) - 1) <= 0)
			sysfatal("read /dev/kbd: %r");
		if(buf[0] == 'c'){
			if(utfrune(buf, Kdel)){
				close(fd);
				threadexitsall(nil);
			}
			if(utfrune(buf, 't'))
				trace = !trace;
		}
		if(buf[0] != 'k' && buf[0] != 'K')
			continue;
		s = buf + 1;
		k = 0xc00;
		while(*s != 0){
			s += chartorune(&r, s);
			switch(r){
			case Kdel: close(fd); threadexitsall(nil);
			case 'c':	k |= 0x0020; break;
			case 'x':	k |= 0x0010; break;
			case 'z':	k |= 0x1000; break;
			case 10:	k |= 0x2000; break;
			case Kup:	k |= 0x0101; break;
			case Kdown:	k |= 0x0202; break;
			case Kleft:	k |= 0x0004; break;
			case Kright:	k |= 0x0008; break;
			case Kesc:
				if(paused)
					qunlock(&pauselock);
				else
					qlock(&pauselock);
				paused = !paused;
				break;
			}
		}
		keys = ~k;
	}
}

void
threadmain(int argc, char **argv)
{
	scale = 1;
	ARGBEGIN{
	case '2':
		scale = 2;
		break;
	case '3':
		scale = 3;
		break;
	default:
		;
	} ARGEND;
	
	if(argc != 1){
		fprint(2, "usage: %s rom", argv0);
		threadexitsall("usage");
	}
	loadrom(*argv);
	if(initdraw(nil, nil, nil) < 0)
		sysfatal("initdraw: %r");
	proccreate(keyproc, nil, 8192);
	mc = initmouse(nil, screen);
	if(mc == nil)
		sysfatal("initmouse: %r");
	screeninit();
	cpureset();
	vdpmode();
	for(;;){
		if(paused != 0){
			qlock(&pauselock);
			qunlock(&pauselock);
		}
		if(dma != 1)
			step();
		if(dma != 0)
			dmastep();
		z80step();
		vdpstep();
	}
}

void
flush(void)
{
	extern uchar pic[320*224*2*3*3];
	Mouse m;
	Rectangle r;
	uchar *s;
	int w;


	if(nbrecvul(mc->resizec) > 0){
		if(getwindow(display, Refnone) < 0)
			sysfatal("resize failed: %r");
		screeninit();
	}
	while(nbrecv(mc->c, &m) > 0)
		;
	if(scale == 1){
		loadimage(tmp, tmp->r, pic, 320*224*4);
		draw(screen, picr, tmp, nil, ZP);
	}else{
		s = pic;
		r = picr;
		w = 320*4*scale;
		while(r.min.y < picr.max.y){
			loadimage(tmp, tmp->r, s, w);
			s += w;
			r.max.y = r.min.y+scale;
			draw(screen, r, tmp, nil, ZP);
			r.min.y = r.max.y;
		}
	}
	flushimage(display, 1);
}