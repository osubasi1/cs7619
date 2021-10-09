#include <iostream>
#include <signal.h>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <csignal>
#include <stdlib.h>
#include "Server.h"

struct laptop_information{
	int customer_id;
	int order_number;
	int laptop_type;
	int engineer_id = 0;
	int expert_id = 0;

	std::string toString(){
			return to_string(customer_id) + " " + to_string(order_number) + " " +
			to_string(laptop_type) + " " + to_string(engineer_id) + " " +
			to_string(expert_id);
	};
};

laptop_information laptop_info;
Server server;
pthread_t msg1[MAX_CLIENT];
int num_message = 0;

void close_app(int s) {
	server.closed();
	exit(0);
}


laptop_information laptopInfo(string s){

vector<string> v;
string temp = "";
for(int i=0;i<s.length();++i){

	if(s[i]==' '){
		v.push_back(temp);
		temp = "";
	}
	else{
		temp.push_back(s[i]);
	}

}
v.push_back(temp);
laptop_info.customer_id = atoi(v.at(0).c_str());
laptop_info.order_number = atoi(v.at(1).c_str());
laptop_info.laptop_type = atoi(v.at(2).c_str());

return laptop_info;
}


class ServerStub
{

	public:

		int Init(int port, vector<int> opts)
		{
			return server.setup(port, opts);
		}

		void accepted()
		{
			server.accepted();
		}

		static void * received(void * m)
		{
			pthread_detach(pthread_self());
			vector<descript_socket*> desc;
			while(1)
			{
				desc = server.getMessage();
				for(unsigned int i = 0; i < desc.size(); i++) {
					if( desc[i]->message != "" )
					{
						if(!desc[i]->enable_message_runtime)
						{
							desc[i]->enable_message_runtime = true;
							if( pthread_create(&msg1[num_message], NULL, ServerStub::send_client, (void *) desc[i]) == 0) {
							cerr << "ACTIVATED THREAD SEND MESSAGES" << endl;
							}
							num_message++;
						}

						string rec_info = desc[i]->message;
						laptopInfo(rec_info);
						cout << "\nReceived laptop info is: " <<laptop_info.toString()<< endl;

						// cout << "id:      " << desc[i]->id      << endl
						//      << "ip:      " << desc[i]->ip      << endl
						//      << "message: " << desc[i]->message << endl
						//      << "socket:  " << desc[i]->socket  << endl
						//      << "enable:  " << desc[i]->enable_message_runtime << endl;
						server.clean(i);
					}
				}
				usleep(1000);

			}
			return 0;
		}

		static void * send_client(void * m)
		{
				struct descript_socket *desc = (struct descript_socket*) m;

				if(!server.is_online() && server.get_last_closed_sockets() == desc->id) {
					cerr << "Connection closed:( id:" << desc->id << " ip:" << desc->ip << " )"<< endl;
				}
				server.Send(laptop_info.toString(), desc->id);
				pthread_exit(NULL);
		}
};

int main(int argc, char **argv)
{
	if(argc < 2) {
		return 0;
	}
	if(argc == 3)
	std::signal(SIGINT, close_app);

	ServerStub serverStub;
	pthread_t msg;
  vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };

	if( serverStub.Init(atoi(argv[1]),opts) == 0) {
		if( pthread_create(&msg, NULL, serverStub.received, (void *)0) == 0)
		{
			while(1) {
				serverStub.accepted();
				cerr << "Accepted" << endl;
			}
		}
	}
	else
		cerr << "Error during socket" << endl;
	return 0;
}
