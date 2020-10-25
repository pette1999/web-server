#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// got help from Gabriela on this method
char *parse(char *input)
{
  // Find the first 'space' in the command line, which would be the start of the port number
  char *start_of_path = strchr(input, ' ') + 1;
  char *start_of_query = strchr(start_of_path, ' ');

  char path[start_of_query - start_of_path];

  //copy the string into the memory
  strncpy(path, start_of_path, start_of_query - start_of_path);

  // define the null term
  path[sizeof(path)] = 0;

  return path;
}

void *client_handler(void *arg)
{
  char msg[80];
  int sockfd;
  sockfd = *(int *)arg;
  char *path;
  char slash = '/';
  char* tempchar;
  char *httpOK = "HTTP/1.1 200 OK\r\n";
  char *httpNotFound = "HTTP/1.1 404 Not Found\r\n";
  char notFound[] = "<html><head><title>Simple 404 Error Page Design</title><link href=\"https://fonts.googleapis.com/css?family=Roboto:700\" rel=\"stylesheet\"><style>h1 {font-size: 80px;font-weight: 800;text-align: center;font-family: 'Roboto', sans-serif;}h2 {font-size: 25px;text-align: center;font-family: 'Roboto', sans-serif;margin-top: -40px;}p {text-align: center;font-family: 'Roboto', sans-serif;font-size: 12px;}.container {width: 300px;margin: 0 auto;margin-top: 15%;}</style></head><body><div class=\"container\"><h1>404</h1><h2>Page Not Found</h2><p>The Page you are looking for doesn't exist or an other error occured. Go to <a href=\"https://www.google.com\">Home Page.</a></p></div></body></html>";

  // allocates the requested memory and returns a pointer to tempchar
  tempchar = (char *)malloc(sizeof(char));

  if (read(sockfd, msg, 80) > 0)
  {
    msg[strlen(msg) - 1] = '\0';
    printf("%s\n", msg);

    path = parse(msg);
    printf("Path:%s\n", path);

    //set the default filename to index.html
    if((char)path[0] == slash)
    {
      tempchar = "index.html";
    }
    else
    {
      // copy the file name to teh new variable tempchar
      for (int i = 0; i < 80; i++)
      {
        if (path[i + 1] == '\0')
        {
          break;
        }
        tempchar[i] = path[i + 1];
      }
    }

    printf("Filename:%s\n", tempchar);
    //dettermine if target file exists = index.html
    // access(): checks whether the calling process can access the file pathname
    if (access(tempchar, F_OK) != -1)
    { 
      // file exists
      printf("%s\n", "The file exists");

      // open the file with permission read only
      FILE *f = fopen(tempchar, "r");
      if (!f)
      {
        perror("The file was not opened\n");
        write(sockfd, httpNotFound, strlen(httpNotFound));
        // customed 404 not found page
        write(sockfd, notFound, strlen(notFound));
        write(sockfd, "\n", 1);
      }
      else
      {
        printf("%s\n", "The file opened");
        write(sockfd, httpOK, strlen(httpOK));

        char data[80];
        while (fgets(data, 80, f) != NULL)
        {
          /* echo message back to client */
          write(sockfd, data, strlen(data));
        }

        fclose(f);
      }
    }
    else
    { 
      // file doesn't exist
      printf("%s\n", "The file does not exist");
      write(sockfd, httpNotFound, strlen(httpNotFound));
      // customed 404 not found page
      write(sockfd, notFound, strlen(notFound));
      write(sockfd, "\n", 1);
    }
  }

  close(sockfd);
}

int main(int argc, char *argv[])
{

  const int backlog = 5;

  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  pthread_t tid;

  int sockfd, client_sockfd;
  int serverlen, clientlen;

  if (argc != 3)
  {
    printf("Usage: %s <ip-address> <port> \n", argv[0]);
    return -1;
  }

  /* Create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    perror("Could not create socket");
    return -1;
  }

  /* Name the socket */
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  server_addr.sin_port = htons(atoi(argv[2]));

  /* bind to server socket */
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    perror("Could not bind to socket");
    close(sockfd);
    return -1;
  }

  /* wait for client to connect */
  listen(sockfd, backlog);

  while (1)
  {
    /* Accept a connection */
    clientlen = sizeof(client_addr);
    client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clientlen);
    if (client_sockfd == -1)
    {
      perror("Unable to accept client connection request");
      continue;
    }

    if (pthread_create(&tid, NULL, client_handler, (void *)&client_sockfd) < 0)
    {
      perror("Unable to create client thread");
      break;
    }
  }

  close(sockfd);

  return 0;
}