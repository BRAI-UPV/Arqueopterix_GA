
#ifndef __SOCKETQ4S_H__
#define __SOCKETQ4S_H__

//#define SOCKETQ4SImport   __declspec( dllimport )
#define SOCKETQ4SExport   __declspec( dllexport )

#include <vector>
using namespace std;

SOCKETQ4SExport void *test_reconfig(void *);
SOCKETQ4SExport void *InitControlSocketThread(void *);
//SOCKETQ4SExport void InitControlSocket();

void split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);
void Reconfigure(int crf, int framerate, int bitrate, int buffsize, int width, int height);

#endif	/* __SOCKETQ4S_H__ */


/*EJEMPLO DE THREAD
#include <pthread.h>
//Socket de control de calidad
pthread_t configSock;
if(pthread_create(&configSock, NULL, init_control_socketOLD, NULL) != 0) {
	ga_error("cannot create CONFIG CONTROL SOCKET\n");
	return -1;
}
pthread_detach(configSock);*/