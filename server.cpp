#include <signal.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

#define BACKLOG 20
// more than this in the queue, and client connect will fail

void error(char *msg)
{
    cerr << msg << endl;
    exit(-1);
}


sockaddr_in make_server_addr(u_short port)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    return addr;
}

int	create_server_socket(u_short port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in my_addr = make_server_addr(port);
    if (s == -1)
        error("socket()");
    bind(s, (sockaddr*)&my_addr, sizeof my_addr);
    listen(s, BACKLOG);
    return s;
}

void get_file_request(int socket, char *fileName)
{
    char buf[BUFSIZ];
    int n = read(socket, buf, BUFSIZ);
    if (n < 0)
        error("read from socket");
    buf[n] = '\0';
    strcpy(fileName, buf);
    cout << "Server got file name of '" << fileName << "'\n";
}

void write_file_to_client_socket(char *file, int socket)
{
    char buf[BUFSIZ];
    int n;
    int ifd = open(file, O_RDONLY);
    if (ifd == -1)
        error("open()");
    while ((n = read(ifd, buf, BUFSIZ)) > 0)
        write(socket, buf, n);
    close(ifd);
}

void handle_request(int client_socket)
{
    char fileName[BUFSIZ];
    get_file_request(client_socket, fileName);
    write_file_to_client_socket(fileName, client_socket);
    close(client_socket);
}

void time_out(int arg)
{
    cerr << "Server timed out" << endl;
    exit(0);
}

void set_time_out(int seconds)
{
    itimerval value;
    timerclear(&value.it_interval);
    timerclear(&value.it_value);
    value.it_value.tv_sec = seconds;
    setitimer(ITIMER_REAL, &value, NULL);
    signal(SIGALRM, time_out);
}


void accept_client_requests(int server_socket)
{
    int client_socket;
    sockaddr_in client_addr;
    socklen_t sin_size = sizeof client_addr;
    set_time_out(10);
    while (client_socket = accept(server_socket, (sockaddr*)&client_addr, &sin_size))
    {
        set_time_out(10);
        handle_request(client_socket);
    }
}

/*
   Note, I put in a 5 second time out so you don't leave servers running.
   Otherwise, you can easily leave a bunch of servers running.
*/

int main(int argc, char *argv[])
{
    if (argc != 2)
        error("usage: server port");
    u_short port = atoi(argv[1]);
    int server_socket = create_server_socket(port);
    accept_client_requests(server_socket);
    shutdown(server_socket, 2);

}
