
#include <stdio.h>
#ifndef WIN32
  #include <sys/socket.h>
#endif

#include "ga-common.h"
#include "ga-module.h"
#include "encoder-common.h"
#include "comun.h"

#include <string>
#include <sstream>


#define PORT 6015
#define BUFSIZE 2048

SOCKET ctrlsocket = -1; /* our socket */
struct sockaddr_in myaddr;      /* our address */
struct sockaddr_in remaddr;     /* remote address */
int addrlen = sizeof(remaddr);            /* length of addresses */
int recvlen;                    /* # bytes received */
char buf[BUFSIZE];     /* receive buffer */



void * InitControlSocketThread(void *){
	/* create a UDP socket */
	ctrlsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(ctrlsocket < 0) {
		ga_error("##### MYSOCKET: Controller socket-init: %s\n", strerror(errno));
		return NULL;
	}

	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(ctrlsocket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		ga_error("MYSOCKET: Socket Bind failed\n");
		return NULL;
	}
	ga_log("##### MYSOCKET: Socket de control iniciado correctamente\n");

    /* now loop, receiving data and printing what we received */
	while(1) {
		ga_log("##### MYSOCKET: waiting on port %d\n", PORT);
        fflush(stdout);
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFSIZE);
		if ((recvlen = recvfrom(ctrlsocket, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen)) == SOCKET_ERROR){
            printf("MYSOCKET: recvfrom() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        //recvlen = recvfrom(ctrlsocket, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        ga_log("##### MYSOCKET: received %d bytes\n", recvlen);
		fprintf(stderr, "MYSOCKET: received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = '\0';
            ga_log("***MYSOCKET***: received message: \"%s\", Length: %i\n", buf, recvlen);
			string str(buf);
			vector<string> valores = split(str, ' ');
			if (valores.size()>0){
				ga_log("##### MYSOCKET: Comparamos: %sFIN\n", valores[0].c_str());
				if (valores[0].compare("EXIT")==0) break;
				else if (valores[0].compare("SET_CRF")==0){
					try{
						int crf = stoi(valores[1]);
						Reconfigure(crf, -1, -1, -1, -1, -1);
					}catch(...){}
				}
				else if (valores[0].compare("SET_FRAMERATE")==0){
					try{
						int framerate = stoi(valores[1]);
						Reconfigure(-1, framerate, -1, -1, -1, -1);
					}catch(...){}
				}
				else if (valores[0].compare("SET_BITRATE")==0){
					try{
						int bitrate = stoi(valores[1]);
						Reconfigure(-1, -1, bitrate, -1, -1, -1);
					}catch(...){}
				}
				else if (valores[0].compare("SET_BUFFERSIZE")==0){
					try{
						int buffsize = stoi(valores[1]);
						Reconfigure(-1, -1, -1, buffsize, -1, -1);
					}catch(...){}
				}
				else if (valores[0].compare("SET_WIDTH")==0){
					try{
						int width = stoi(valores[1]);
						Reconfigure(-1, -1, -1, -1, width, -1);
					}catch(...){}
				}
				else if (valores[0].compare("SET_HEIGHT")==0){
					try{
						int height = stoi(valores[1]);
						Reconfigure(-1, -1, -1, -1, -1, height);
					}catch(...){}
				}
			}
        }
    }
	close(ctrlsocket);
	ga_log("##### MYSOCKET: Socket de control finalizado\n");

}


void split(const string &s, char delim, vector<string> &elems) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

void Reconfigure(int crf, int framerate, int bitrate, int buffsize, int width, int height){
	ga_log("##### Reconfigure->Bitrate: %i, Framerate: %i\n", bitrate, framerate);
	int err;
	ga_ioctl_reconfigure_t reconf;
	if(encoder_running() != 0) {
		ga_log("##### Encoder running\n");
		bzero(&reconf, sizeof(reconf));
		reconf.id = 0;

		if (bitrate>0){
			reconf.bitrateKbps = bitrate;
			reconf.bufsize = 5 * bitrate / 24;
		}
		if (framerate>0){
			reconf.framerate_n = framerate;
			reconf.framerate_d = 1;
		}
		if (buffsize>0){
			reconf.bufsize = buffsize;
		}
		ga_log("##### Get encoder?");
		if(encoder_get_vencoder()->ioctl) {
			fprintf(stderr, "... OK\n");
			ga_log("... OK\n");
			err = encoder_get_vencoder()->ioctl(GA_IOCTL_RECONFIGURE, sizeof(reconf), &reconf);
			if(err < 0) {
				ga_error("##### Reconfigure bitrate encoder failed, err = %d.\n", err);
				ga_module_stop(encoder_get_vencoder(), NULL);
			} else {
				ga_error("##### Reconfigure bitrate encoder OK, bitrate=%d; bufsize=%d; framerate=%d/%d.\n",
						reconf.bitrateKbps, reconf.bufsize,
						reconf.framerate_n, reconf.framerate_d);
			}
		}
	}
}

void *
test_reconfig(void *) {
//#ifndef TEST_RECONFIGURE
//	return NULL;
//#endif
	int s = 0, err;
	int kbitrate[] = { 1000, 500, 8000 };//{ 2000, 8000 };
	int framerate[][2] = { { 2, 1 }, {60, 1}, {2, 1} };
	//int widhtHeight[][2] = { { 800, 600 }, {640, 480}, {320, 240} };
	ga_error("reconfigure thread started ...\n");

	//FILE * pFile = fopen ("salidaReconf.txt","w");
	//fprintf(stderr, "##### Reconfigure thread started ...\n");

	while(1) {
		ga_ioctl_reconfigure_t reconf;
		if(encoder_running() == 0) {
#ifdef WIN32
			Sleep(1);
#else
			sleep(1);
#endif
			continue;
		}
#ifdef WIN32
		Sleep(10 * 1000);
#else
		sleep(20);
#endif
		bzero(&reconf, sizeof(reconf));
		reconf.id = 0;
//#if 1
		reconf.bitrateKbps = kbitrate[s%3];
		reconf.bufsize = 5 * kbitrate[s%3] / 24;
//#endif
		reconf.framerate_n = framerate[s%3][0];
		reconf.framerate_d = framerate[s%3][1];

		//reconf.width = widhtHeight[s%3][0];
		//reconf.height = widhtHeight[s%3][1];

		// encoder
		if(encoder_get_vencoder()->ioctl) {
			err = encoder_get_vencoder()->ioctl(GA_IOCTL_RECONFIGURE, sizeof(reconf), &reconf);
			if(err < 0) {
				ga_error("reconfigure encoder failed, err = %d.\n", err);
				fprintf(stderr, "Reconfigure bitrate encoder failed, err = %d.\n", err);
				ga_module_stop(encoder_get_vencoder(), NULL);
			} else {
				ga_error("reconfigure encoder OK, bitrate=%d; bufsize=%d; framerate=%d/%d.\n",
						reconf.bitrateKbps, reconf.bufsize,
						reconf.framerate_n, reconf.framerate_d);
				fprintf(stderr, "Reconfigure encoder OK, bitrate=%d; bufsize=%d; framerate=%d/%d.\n",
						reconf.bitrateKbps, reconf.bufsize,
						reconf.framerate_n, reconf.framerate_d);
			}
		}
		s = (s + 1) % 6;
	}
	
	//fclose (pFile);
	return NULL;
}