#ifndef _NETINET_TCP_H
#define _NETINET_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#define TCP_ACKDELAYTIME   0x2001
#define TCP_NOACKDELAY     0x2002
#define TCP_MAXSEG         0x2003
#define TCP_NODELAY        0x2004

#define TCP_KEEPALIVE      0x0000 /* ? */
#define TCP_KEEPINTVL      0x0000 /* ? */
#define TCP_KEEPCNT        0x0000 /* ? */

#ifdef __cplusplus
}
#endif

#endif /* _NETINET_TCP_H */
