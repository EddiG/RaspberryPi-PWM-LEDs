/* simple-server.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** simple-server.c                                                       ***/
/***                                                                       ***/
/*****************************************************************************/

/**************************************************************************
*	This is a simple echo server.  This demonstrates the steps to set up
*	a streaming server.
**************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include "ws2812-RPi.h"

#define MY_PORT		9999
#define MAXBUF    512
#define LEDS_COUNT 90
#define BYTES_FOR_LED 3

void verifyData(unsigned char data[], ssize_t size);

int main(int Count, char *Strings[])
{   
  int sockfd;
	struct sockaddr_in self;
	unsigned char buffer[MAXBUF];

	/*---Create streaming socket---*/
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize address/port structure---*/
	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
	self.sin_addr.s_addr = INADDR_ANY;

	/*---Assign a port number to the socket---*/
  if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )	{
		perror("socket--bind");
		exit(errno);
	}

	/*---Make it a "listening socket"---*/
	if ( listen(sockfd, 20) != 0 ) {
		perror("socket--listen");
		exit(errno);
	}

	/*---Forever... ---*/
  int clientfd;
	struct sockaddr_in client_addr;
	int addrlen=sizeof(client_addr);
  ssize_t dataSize;

	/*---accept a connection (creating a data pipe)---*/
	clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
	printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  /*---Initialize PWM on Raspberry Pi---*/
  init();

  /*---Echo back anything sent---*/
  while (1) {
    dataSize = recv(clientfd, buffer, MAXBUF, 0); 
    if (dataSize > 0)
      verifyData(buffer, dataSize); 
  }

  /*---Terminate PWM on Raspberry Pi---*/
  dispose();

	/*---Close data connection---*/
	close(clientfd);

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}

void verifyData(unsigned char data[], ssize_t size) 
{
  unsigned int i;

  if (LEDS_COUNT*BYTES_FOR_LED > size) {
    printf("Need more data size. Now recive %zu", size);
    return;
  }

  for (i=0; i<LEDS_COUNT; i++) {
    setPixelColor(i, data[i], data[i+1], data[i+2]);
  }

  show();
}
