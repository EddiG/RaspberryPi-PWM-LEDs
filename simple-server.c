#include <stdio.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include "ws2812-RPi.h"

#define MY_PORT		9999
#define MAXBUF    512
#define LEDS_COUNT 90
#define BYTES_FOR_LED 3

void send_pwm(unsigned char data[], ssize_t size) 
{
  unsigned int i;

  if (LEDS_COUNT*BYTES_FOR_LED > size) {
    printf("Few data. Now recive %zu", size);
    return;
  }

  for (i=0; i<LEDS_COUNT; i++) {
    printf("%x %x %x ", data[i], data[i+1], data[i+2]);
    setPixelColor(i, data[i], data[i+1], data[i+2]);
  }

  show();
}

int main(int argc, char *argv[])
{   
  int sockfd;
	struct sockaddr_in self;
	unsigned char buffer[MAXBUF];

	/*---Create streaming socket---*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize address/port structure---*/
	memset(&self, 0, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
	self.sin_addr.s_addr = INADDR_ANY;

	/*---Assign a port number to the socket---*/
  if (bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0)	{
		perror("socket--bind");
		exit(errno);
	}

	/*---Make it a "listening socket"---*/
	if (listen(sockfd, 20) != 0) {
		perror("socket--listen");
		exit(errno);
	}

	/*---Forever... ---*/
  int clientfd;
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
  ssize_t dataSize;

	/*---accept a connection (creating a data pipe)---*/
	clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
	printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  /*---Initialize PWM on Raspberry Pi---*/
  init_pwm();

  /*---Echo back anything sent---*/
  while (1) {
    dataSize = recv(clientfd, buffer, MAXBUF, MSG_WAITALL); 
    if (dataSize > 0)
      send_pwm(buffer, dataSize);
  }

  /*---Terminate PWM on Raspberry Pi---*/
  dispose_pwm();

	/*---Close data connection---*/
	close(clientfd);

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}

