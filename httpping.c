#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define MAX_PING 30


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// The time difference in milliseconds */
double msecs (struct timeval * t2, struct timeval * t1)
{  
  /* Compute delta in second, 1/10's and 1/1000's second units */
  double delta_seconds= (double)t2 -> tv_sec - (double)t1 -> tv_sec;
  double delta_milliseconds= ((double)t2 -> tv_usec - (double)t1 -> tv_usec) / 1000.0; 

  if(delta_milliseconds < 0.0)
    { /* manually carry a one from the seconds field */
      delta_milliseconds += 1000.0;                              /* 1e3 */
      -- delta_seconds;
    }
  return (delta_seconds * 1000.0) + delta_milliseconds;
}
 

int main(int argc, char *argv[])
{
	
	int cont=0;
    int p=0;
    double teste[30];
    double mev, mav, med, dp, soma;
    int sockfd=0, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct timeval http_time_snd;
	struct timeval http_time_rec;
	char pagina[256] = "/";
	int c,i,zabbix, bytes=1, print_buffer=0;
	char HTTPD_200[] = "HTTP/1.1 200";
	char HTTPD_301[] = "HTTP/1.1 301";
	char HTTPD_400[] = "HTTP/1.1 400";
	char HTTPD_404[] = "HTTP/1.1 404";
	char HTTPD_408[] = "HTTP/1.1 408";
	char HTTPD_500[] = "HTTP/1.1 500";
	char HTTPD_502[] = "HTTP/1.1 502";
  	char HTTPD_503[] = "HTTP/1.1 503";
	char HTTPD_504[] = "HTTP/1.1 504";
	char buffer[256];
    char bufferRec[256];
	char httpmsg[256];
 	char httpserver[256];   	
 	cont = 5;
 	zabbix = 0;
 	portno = 80;

	while((c = getopt(argc, argv, "h:p:c:w:z:b")) != EOF)
	{
		switch (c)
		{
			case 'h':
				strcpy(httpserver,optarg);
				break;
			case 'p':
				portno = atoi(optarg);
				break;	
			case 'c':
				cont = atoi(optarg);
				if(cont > MAX_PING){
				printf("Number of pings Maximum = 30 \n");
				}
				break;
			case 'w':
				strcpy(pagina,optarg);
				break;
			case 'z':
				zabbix = 1;
				break;
			case 'b':
				print_buffer=1;
				break;
			default :
				printf("USAGE: HTTPPING \n\t-h Hostname \n\t-p Port \n\t-c Number of pings Maximum 30 \n\t-w Enter the Page \n\t-z Exit to the Zabbix  \n\t-b Print server Response\n ");
				
			return 0;		
	}

		}
	

	if (argc < 3) {
     		fprintf(stderr,"use %s server_web port\n", argv[0]);
       		exit(0);
    	}

    
    	if (sockfd < 0) 
        	error("ERROR opening socket");
    	server = gethostbyname(httpserver);
    	if (server == NULL) {
   	 	fprintf(stderr,"ERROR, no such host\n");
       	exit(0);
    	}
        
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
   	bcopy((char *)server->h_addr, 
         	(char *)&serv_addr.sin_addr.s_addr,
         	server->h_length);
    	serv_addr.sin_port = htons(portno);
              

	//most, lower, average, standard deviation 
        mev= 10000.00;
        mav= -1.00;
        soma= 0.00;
        med= 0.00;
        dp= 0.00;
   
      
   
    	bzero(buffer,256);
    	strcpy(httpmsg,"GET ");
	strcat(httpmsg , pagina);
	strcat(httpmsg , " HTTP/1.1\r\nhost:");
	strcat(httpmsg , httpserver);
	strcat(httpmsg ,"\r\n\r\n");
		if(zabbix == 0)
			printf("http msg: %s\n",httpmsg);
  
       	for(i=0;i<cont;i++){
        	gettimeofday(&http_time_snd, NULL);
         		
        	
        	sockfd = socket(AF_INET, SOCK_STREAM, 0);
        		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        			error("ERROR connecting");
			n = write(sockfd,httpmsg,strlen(httpmsg));
			if (n < 0) 
				error("ERROR writing to socket");
				
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0) 
				error("ERROR reading from socket");
			
			if(strstr(buffer, HTTPD_200) == NULL) {
				
				if(strstr(buffer, HTTPD_301) != NULL) {
					printf("301 MOVED PERMANENTLY,Check the page and try again.\n");
				}else if(strstr(buffer, HTTPD_404) != NULL) {
					printf("404 NOT FOUND,Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_400) != NULL) {
					printf("400 BAD REQUEST, Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_408) != NULL) {
					printf("408 REQUEST TIMEOUT, Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_500) != NULL) {
					printf("500 INTERNAL SERVER ERROR, Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_502) != NULL) {
					printf("502 BAD GATEWAY, Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_503) != NULL) {
					printf("503 SERVICE UNAVAILABLE, Check the page and try again.\n");
				}
				else if(strstr(buffer, HTTPD_504) != NULL) {
					printf("504 GATEWAY TIMEOUT, Check the page and try again.\n");
				}
					else{
					printf("Problem with the page, Check the page and try again.\n");
				}
				return 0;
			}
			
			close(sockfd);
        	gettimeofday(&http_time_rec, NULL);		
			if(print_buffer==1){
				printf("\nServer HTTP Reply\n\n");			
				printf("%s\n\n",buffer);}
        	double http_RTT = msecs(&http_time_rec, &http_time_snd);
			teste[i]=http_RTT;

            if(zabbix == 0)
				printf("%d - HTTP RTT =  %3.2f ms\n",(i+1),http_RTT);
                 
            sleep(1);
   
  	         
        	 //Calculating lower value
     
			if(teste[i] < mev) {
    			mev=teste[i];
			}
                //calculating higher value
			if(teste[i] > mav) {
             		mav=teste[i];
            }
 		//calculating average
           	 soma=soma+teste[i];

	}
				
 	
 			
	med=soma/cont;               
    	soma=0;
    
	//calculating standard deviation
 	for(p=0;p<cont;p++){
    	
        soma= soma+pow(teste[p]-med,2);
    }
    dp= sqrt(soma/cont-1);
    if(zabbix == 0){
		
		printf("\n==========================\n");
		printf(" PING statistics:\n");
		printf(" Minimum  = %3.2fms\n Maximum = %3.2fms\n Average = %3.2fms\n Standard Deviation = %3.2fms\n",mev,mav,med,dp);
		printf("\n");
	}else{
		printf("%3.2f\n",med);
	}

    
    	return 0;
}




