#include <iostream>
#include <signal.h>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include "Client.h"

using namespace std;
mutex mtx;
vector<int> time_collector;

struct order_details{
	int customer_id     = 1000;
	int order_number    = 0;
	int laptop_type     = 0;

	std::string toString(){
		return to_string(customer_id) + " " + to_string(order_number) + " " +
					 to_string(laptop_type);
	};
};

struct laptop_information{
	int customer_id = 1000;
	int order_number = 0;
	int laptop_type = 0;
	int engineer_id = 0;
	int expert_id = 0;

	std::string toString(){
			return to_string(customer_id) + " " + to_string(order_number) + " " +
			to_string(laptop_type) + " " + to_string(engineer_id) + " " +
			to_string(expert_id);
	};
};

// this variable is used multiple places
order_details order;

class ClientStub
{
  Client client;
  public:

    void Init(string ip, int port)
    {

      client.setup(ip, port);
    }

    string Order(order_details order)
    {
			auto start = chrono::high_resolution_clock::now();

			string rec;
			client.Send(order.toString());
			// cout << " \nSend order information: " << order.toString();
			cout << endl;
      rec = client.receive();
      if( rec != "" )
      {
        cout << " \nReceived order information: " << rec;
        cout << endl;
				client.exit();

      }
			auto end = chrono::high_resolution_clock::now();
			double time_taken =
			chrono::duration_cast<chrono::nanoseconds>(end - start).count();
			// mtx.unlock();
			time_collector.push_back(time_taken);
			return rec;
    }


		void exit()
		{
			client.exit();
		}

};



void sig_exit(int s)
{
	exit(0);
}
int main(int argc, char *argv[])
{


	if(argc != 6) {
		cerr << "Please check the parameters" << endl;
		return 0;
	}
	int num_customers = atoi(argv[3]);
	int num_orders = atoi(argv[4]);
	// starting costomer id from 1000 and auto increase it to have unique
	order.customer_id = 1000;
	order.laptop_type = atoi(argv[5]);

	vector<thread> threads;
	vector<ClientStub> cStub;


	// initializing the customers
	for (int i = 0; i <num_customers; i++)
	{
		ClientStub client;
		client.Init(argv[1],atoi(argv[2]));
		cStub.emplace_back(client);
	}
	// costomer threads
	for (int i = 0; i < num_customers; i++)
	{
		order.customer_id++;
		order.order_number = 0;
		for (int j = 0; j < num_orders; j++)
		{
			order.order_number++;
			threads.push_back(std::thread(&ClientStub::Order, &cStub.at(i), order));
		}
	}

	for (auto &th : threads)
	{
		if(th.joinable()){
		th.join();
		}
	}
	cout << "\nsize of time vector is: " << time_collector.size()<<endl;

	return 0;
}
