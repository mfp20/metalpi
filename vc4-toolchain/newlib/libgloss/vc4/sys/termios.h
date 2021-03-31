#ifndef TERMIOS_H
#define TERMIOS_H 1

typedef unsigned int tcflag_t;
typedef unsigned char cc_t;

#define NCCS 1

struct termios
{
  tcflag_t c_iflag;
  tcflag_t c_oflag;
  tcflag_t c_cflag;
  tcflag_t c_lflag;
  cc_t c_cc[NCCS];
};

/* c_iflag bits.  */
#define IGNBRK   0x0001
#define BRKINT   0x0002
#define IGNPAR   0x0004
#define PARMRK   0x0008
#define INPCK    0x0010
#define ISTRIP   0x0020
#define INLCR    0x0040
#define IGNCR    0x0080
#define ICRNL    0x0100
#define IUCLC    0x0200
#define IXON     0x0400
#define IXANY    0x0800
#define IXOFF    0x1000
#define IMAXBEL  0x2000
#define IUTF8    0x4000

/* c_oflag bits.  */
#define OPOST  0x0001
#define OLCUC  0x0002
#define ONLCR  0x0004
#define OCRNL  0x0008
#define ONOCR  0x0010
#define ONLRET 0x0020
#define OFILL  0x0040
#define OFDEL  0x0080

/* c_lflag bits.  */
#define ISIG   0x0001
#define ICANON 0x0002
#define ECHO   0x0004
#define ECHOE  0x0008
#define ECHOK  0x0010
#define ECHONL 0x0020
#define NOFLSH 0x0040
#define TOSTOP 0x0080
#define IEXTEN 0x0100

/* c_cflag bits.  */
#define CS8    0x0001

/* tcsetattr actions.  */
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

extern int tcgetattr(int, struct termios *);
extern int tcsetattr(int, int, const struct termios *);

#endif
