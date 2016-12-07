
#ifndef __COMUN_H__
#define __COMUN_H__

//#define COMUNImport   __declspec( dllimport )
#define COMUNExport   __declspec( dllexport )

#include <vector>
using namespace std;

COMUNExport void *test_reconfig(void *);
COMUNExport void *InitControlSocketThread(void *);
//COMUNExport void InitControlSocket();

void split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);
void Reconfigure(int crf, int framerate, int bitrate, int buffsize, int width, int height);

#endif	/* __COMUN_H__ */


/*EJEMPLO DE THREAD
#include <pthread.h>
//Socket de control de calidad
pthread_t configSock;
if(pthread_create(&configSock, NULL, init_control_socketOLD, NULL) != 0) {
	ga_error("cannot create CONFIG CONTROL SOCKET\n");
	return -1;
}
pthread_detach(configSock);*/