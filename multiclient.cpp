#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include "Timer.h" 
using namespace std;
#define N_THREADS 10

char * files[] = {
"/usr/share/dict/words",
"/usr/include/asm-x86_64/unistd.h",
"/usr/include/xulrunner-2/nsIDOMHTMLTextAreaElement.h",
"/usr/include/xulrunner-2/gfxContext.h",
"/usr/include/boost/test/test_tools.hpp",
"/usr/include/gssapi/gssapi.h",
"/usr/include/xf86drm.h",
"/usr/include/kde/ksocketaddress.h",
"/usr/include/kde/kpropertiesdialog.h",
"/usr/include/c++/4.1.1/bits/stl_function.h",
"/usr/include/linux/ixjuser.h",
"/usr/include/wx-2.8/wx/html/htmlcell.h",
"/usr/include/wx-2.8/wx/gdicmn.h",
"/usr/include/SDL/SDL_mixer.h"
};

#define files_length() (sizeof files / sizeof files[0])

void error(char *msg)
{
    perror(msg);
    exit(-1);
}

sockaddr_in make_server_addr(char *host, u_short port)
{
    sockaddr_in addr;
    bzero(&addr, sizeof addr);
    hostent *hp = gethostbyname(host);
    if (hp == 0)
    {
        herror(host);
        exit(-1);
    }
    addr.sin_family = AF_INET;
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    return addr;
}

int connect_socket(char *host, u_short port)
{
    int status;
    int tries = 3;
    sockaddr_in addr = make_server_addr(host, port);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
        error("socket()");
    status = connect(s, (sockaddr*)&addr, sizeof addr);
    if (status < 0)
	{
        perror("connect refused");
	}
    return s;

}
void requestFileFromServer(int server_socket, char *file)
{
    int len = strlen(file);
    int n = write(server_socket, file, len);
    if (n != len)
        error("short write");
}

void readFileFromServer(int server_socket, char *file)
{
    char buf[BUFSIZ];
    int n;
    mode_t mode = 0666;
    int ofd = open(file, O_WRONLY | O_CREAT, mode);
    if (ofd == -1)
        perror("open()");
    while ((n = read(server_socket, buf, BUFSIZ)) > 0)
        write(ofd, buf, n);
    close(ofd);
}


struct Thread_data
{
    int id;
    pthread_t thread_id;
    char * host;
    u_short port;
    char path[BUFSIZ];
};

void make_file_name(char *local_name, char *dir, char *original_path)
{
    char *p = rindex(original_path, '/');
    if (!p)
        error("rindex()");
    sprintf(local_name, "%s/%s", dir, p+1);
}

int remoteCopy(Thread_data & data, char * file)
{
    int server_socket = connect_socket(data.host, data.port);
    requestFileFromServer(server_socket, file);
    char local_name[BUFSIZ];
    make_file_name(local_name, data.path, file);
    readFileFromServer(server_socket, local_name);
    close(server_socket);
}

void make_empty_dir_for_copies(Thread_data & data)
{
    mode_t mode = 0777;
    sprintf(data.path, "./Thread_%d", data.id);
    mkdir(data.path, mode);
}

#define N_FILES_TO_COPY files_length() // copy them all
// #define N_FILES_TO_COPY 1 // just copy one when testing

void *thread_work(void *arg)
{
    Thread_data data = *(Thread_data*)arg;
    make_empty_dir_for_copies(data);
    for (int i=0; i<N_FILES_TO_COPY; ++i)
        remoteCopy(data, files[i]);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " host port\n";
        exit(-1);
    }

    char *host = argv[1];
    u_short port = atoi(argv[2]);
    Thread_data thread_args[N_THREADS];
	Timer t;
	double eTime;
	t.start();  
    for (int i = 0; i < N_THREADS; ++i)
    {
        Thread_data & t = thread_args[i];
        t.id = i;
        t.host = host;
        t.port = port;
        pthread_create(&t.thread_id, NULL, thread_work, &t);
		cout << "Creating Thread : " << t.id << endl; 
	}

    for (int i=0; i < N_THREADS; i++)
        pthread_join(thread_args[i].thread_id, NULL);
	t.elapsedUserTime(eTime); 
	cout << eTime << endl; 
    pthread_exit(0);
}
