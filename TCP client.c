/*ÐÕÃû£º¹¨µ¤Óñ Ñ§ºÅ£º13S103061*/
/* TCPdtc.cpp - main, TCPdaytime */
#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>
#include <errno.h>

void	TCPdaytime(const char *, const char *);
void	errexit(const char *, ...);
SOCKET	connectTCP(const char *, const char *);
SOCKET	connectsock(const char *, const char *, const char *);

#define	LINELEN		 128
#define WSVERS		MAKEWORD(2, 0)

/*------------------------------------------------------------------------
 * main - TCP client for DAYTIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*host = "localhost";	/* host to use if none supplied	*/
	char	*service = "daytime";	/* default service port		*/
	WSADATA wsadata;

	switch (argc) {
	case 1:
		host = "localhost";
		break;
	case 3:
		service = argv[2];
		/* FALL THROUGH */
	case 2:
		host = argv[1];
		break;
	default:
		fprintf(stderr, "usage: TCPdaytime [host [port]]\n");
		exit(1);
	}

	if (WSAStartup(WSVERS, &wsadata) != 0)
		errexit("WSAStartup failed\n");
	TCPdaytime(host, service);
	WSACleanup();
	getchar();
	getchar();
	return 0;	/* exit */
}
void errexit(const char *format, ...)
{
	va_list	args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	exit(1);
}

SOCKET
connectTCP(const char *host, const char *service )
{
	return connectsock( host, service, "tcp");
}
SOCKET
connectsock(const char *host, const char *service, const char *transport )
{
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/


	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* Map service name to port number */
	if ( pse = getservbyname(service, transport) )
		sin.sin_port = pse->s_port;
	else if ( (sin.sin_port = htons((u_short)atoi(service))) == 0 )
		errexit("can't get \"%s\" service entry\n", service);

    /* Map host name to IP address, allowing for dotted decimal */
	if ( phe = gethostbyname(host) )
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
		errexit("can't get \"%s\" host entry\n", host);

    /* Map protocol name to protocol number */
	if ( (ppe = getprotobyname(transport)) == 0)
		errexit("can't get \"%s\" protocol entry\n", transport);
    /* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s == INVALID_SOCKET)
		errexit("can't create socket: %d\n", GetLastError());

    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) ==
	    SOCKET_ERROR)
		errexit("can't connect to %s.%s: %d\n", host, service,
			GetLastError());
	return s;
}

/*------------------------------------------------------------------------
 * TCPdaytime - invoke Daytime on specified host and print results
 *------------------------------------------------------------------------
 */
void
TCPdaytime(const char *host, const char *service)
{
	char	buf[LINELEN+1];		/* buffer for one line of text	*/
	SOCKET	s;			/* socket descriptor		*/
	int	cc;			/* recv character count		*/

  	s = connectTCP(host, service);

	cc = recv(s, buf, LINELEN, 0);
	while( cc != SOCKET_ERROR && cc > 0) {
		 buf[cc] = '\0';	/* ensure null-termination	*/
		 (void) fputs(buf, stdout);
		cc = recv(s, buf, LINELEN, 0);
	}
	closesocket(s);
}
