#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux
#ifdef WIN32
#include <winsock2.h>
#include "../mingw_net.h"
#endif // WIN32
#include <iostream>
#include <thread>

using namespace std;

#ifdef WIN32
void perror(const char* msg) { fprintf(stderr, "%s %ld\n", msg, GetLastError()); }
#endif // WIN32

int sd_arr[1000] = {0};
int cnt = 0;

void usage() {
	cout << "echo-server:\n";
	cout << "syntax : echo-server <port> [-e[-b]]\n";
	cout << "sample : echo-server 1234 -e -b\n";
}

struct Param {
	bool echo{false};
	bool broadcast{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				if((strcmp(argv[i+1],"-b") == 0)){
					broadcast = true;
					i++;
					continue;
				}
				continue;
			}
			port = stoi(argv[i]);
		}
		return port != 0;
	}
} param;

void recvThread(int sd) {
	cout << "connected\n";
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	int index = cnt++;

	while (true) {
		sd_arr[index]=sd;
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res << endl;
			perror(" ");
			break;
		}
		buf[res] = '\0';
		cout << buf;
		cout.flush();
		if (param.echo) {
			if(param.broadcast){
				for(int i = 0; i < cnt; i++){
					res = send(sd_arr[i], buf, res, 0);
					if (res == 0 || res == -1) {
						cerr << "send return " << res << endl;
						perror(" ");
						break;
					}
				}
			}
			else{
				res = send(sd, buf, res, 0);
				if (res == 0 || res == -1) {
					cerr << "send return " << res;
					perror(" ");
					break;
				}
			}
		}
	}
	cout << "disconnected\n";
  close(sd);
}

int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

#ifdef WIN32
	WSAData wsaData;
	WSAStartup(0x0202, &wsaData);
#endif // WIN32

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int res;
#ifdef __linux__
	int optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}
#endif // __linux

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}
	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);

		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}
		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
