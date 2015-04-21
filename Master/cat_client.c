/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(){
  int clientSocket;
  char buffer[2000];
  char read_buf[2000];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(3000);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Connect the socket to the server using the address struct ----*/
  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

  /*---- Read the message from the server into the buffer ----*/

  strcpy(buffer, "this is zero test!");
  send(clientSocket, buffer, 2000, 0);
  recv(clientSocket, read_buf, 2000, 0);

  /*---- Print the received message ----*/
  printf("Data received: %s",buffer);   
  int a;

  while(1){
    puts("1: send Data");
    puts("2: hand up");
    scanf("%d\n", &a);

    switch(a) {
      case 1 :
        if (fgets(buffer, sizeof(buffer), stdin) != NULL){
          send(clientSocket, buffer, 2000, 0);        
        }
        break;
      case 2:
        return 0;
        break;
      default:
        break;
    }


  }


  return 0;
}