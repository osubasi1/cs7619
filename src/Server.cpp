#include "Server.h"

char Server::msg[MAXPACKETSIZE];
int Server::num_client;
int Server::last_closed;
bool Server::isonline;
vector<descript_socket*> Server::Message;
vector<descript_socket*> Server::newsockfd;
std::mutex Server::mt;


void* Server::receiveFromClient(void *arg)
{
	int reveivedByte;
	struct descript_socket *desc = (struct descript_socket*) arg;
	pthread_detach(pthread_self());

        cerr << "open client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket<<" send:"<< desc->enable_message_runtime <<" ]" << endl;
	while(1)
	{
		reveivedByte = recv(desc->socket, msg, MAXPACKETSIZE, 0);
		// if there is no error during receiving data
		if(reveivedByte != -1)
		{

			// if client ends the connection then close it
			if(reveivedByte == 0)
			{
			   isonline = false;
			   cerr << "close client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket<<" ]" << endl;
			   last_closed = desc->id;
			   close(desc->socket);

			   if(num_client>0) num_client--;
				 pthread_exit(NULL);
			   break;
			}
			msg[reveivedByte]=0;
			desc->message = string(msg);
	    std::lock_guard<std::mutex> guard(mt);
			Message.push_back( desc );
		}
  }
	if(desc != NULL)
		free(desc);
	cout << "exit thread: " << this_thread::get_id() << endl;
	pthread_exit(NULL);

	return 0;
}

int Server::setup(int port, vector<int> opts)
{
	int enable = 1;
	isonline = false;
	last_closed = -1;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
 	memset(&serverAddress,0,sizeof(serverAddress));

	for(unsigned int i = 0; i < opts.size(); i++) {
		if( (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  (char *)&enable, sizeof(enable))) < 0 )
		{
			cerr << "Error during setsockopt" << endl;
      			return -1;
	  }
	}

	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons(port);

	if((::bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0){
		cerr << "Error binding" << endl;
		return -1;
	}

 	if(listen(sockfd,5) < 0){
		cerr << "Error listening" << endl;
		return -1;
	}
	num_client = 0;
	isonline = true;
	return 0;
}

void Server::accepted()
{
	socklen_t sosize    = sizeof(clientAddress);
	descript_socket *so = new descript_socket;
	so->socket          = accept(sockfd,(struct sockaddr*)&clientAddress,&sosize);
	so->id              = num_client;
	so->ip              = inet_ntoa(clientAddress.sin_addr);
	newsockfd.push_back( so );
	cerr << "accept client[ id:" << newsockfd[num_client]->id <<
	                      " ip:" << newsockfd[num_client]->ip <<
		              	" handle:" << newsockfd[num_client]->socket << " ]" << endl;
	pthread_create(&serverThread[num_client], NULL, &receiveFromClient, (void *)newsockfd[num_client]);
	isonline=true;
	num_client++;
}

vector<descript_socket*> Server::getMessage()
{
	std::lock_guard<std::mutex> guard(mt);
	return Message;
}

void Server::Send(string msg, int id)
{
	send(newsockfd[id]->socket,msg.c_str(),msg.length(),0);
}

int Server::get_last_closed_sockets()
{
	return last_closed;
}

void Server::clean(int id)
{
	Message[id]->message = "";
	memset(msg, 0, MAXPACKETSIZE);
}

string Server::get_ip_addr(int id)
{
	return newsockfd[id]->ip;
}

bool Server::is_online()
{
	return isonline;
}

void Server::detach(int id)
{
	close(newsockfd[id]->socket);
	newsockfd[id]->ip = "";
	newsockfd[id]->id = -1;
	newsockfd[id]->message = "";
}

void Server::closed()
{
	close(sockfd);
}
