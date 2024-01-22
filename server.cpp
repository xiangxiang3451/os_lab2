#include <signal.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r) {
	wasSigHup = 1;
}

int main()
{	
	int sock = 0;
	int listener;

	struct sockaddr_in addr;
	int addrLen = sizeof(addr);
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0) {
    	printf("socket error");
    	exit(EXIT_FAILURE);
	}

	addr.sin_family = AF_INET; //семейство адресов
	addr.sin_port = htons(8080); //номер порта
	addr.sin_addr.s_addr = INADDR_ANY; //адрес

	//явное связывание сокета с адресом
	int addrBind = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
	if(addrBind == -1) { //не подключилось
    	printf("bind error");
    	exit(EXIT_FAILURE);
	}

	listen(listener, 1);
	
	//Регистрация обработчика сигнала
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa); 
	sa.sa_handler = sigHupHandler; 
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	//Блокировка сигнала
	sigset_t blockedMask, origMask;
	sigemptyset(&blockedMask);
	sigemptyset(&origMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

	//Работа основного цикла
	int maxFd;
	fd_set fds;
	while (1) {
    	FD_ZERO(&fds);
    	FD_SET(listener, &fds);

    	if (sock > 0) FD_SET(sock, &fds); //когда есть активное соединение

    	if (sock > listener) maxFd = sock;
		else maxFd = listener;

	    if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) == -1) {
	        if (errno == EINTR) { 
	            if (wasSigHup == 1) { 
	                wasSigHup = 0;
					continue;
	            }
	        }
	        else break;
	    }

    	
	    	if (sock > 0 && FD_ISSET(sock, &fds)) { //ISSET проверяет, есть ли sock в fds
	        	char buffer[1024] = { 0 };
	        	int bytesRead = read(sock, buffer, 1024); //сюда считываем текст
	        	if (bytesRead > 0)
	            	printf("Received data: %d bytes\n", bytesRead);
	        	else 
					if (bytesRead == 0) {
		            	// Соединение закрыто клиентом
		            	printf("сlient disconnected.\n");
		            	close(sock);
						sock = 0;
		        	}
		        	else {
		            	// Ошибка при чтении данных
		            	printf("read error");
		            	break;
		        	}
				continue;
	    	}
	
		if (FD_ISSET(listener, &fds)) {   // если не остался listener
	        sock = accept(listener, (struct sockaddr*)&addr, (socklen_t*)&addrLen);
	        if (sock < 0) {
	            printf("accept error");
	            break;
	        }
			printf("New connection.\n");
	    }
	}
	close(listener);
	return 0;
}
