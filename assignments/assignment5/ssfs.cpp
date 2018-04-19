#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

typedef struct Command 
{
	string command;
	string file_name;
	string unix_file;
	char character;
	int start_byte;
	int num_bytes;
} Command;

vector<Command> commands;
mutex command_mutex, done_mutex;
condition_variable cond;
int threads_finished = 0;

void execute_commands(int id) 
{
	
}

int main(int argc, char **argv) 
{
	string op1, op2, op3, op4, disk_name;
	switch(argc) 
	{
		case 1:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;
		case 2:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;
		case 3:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
		case 4:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
		case 5:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
			op3 = string(argv[4]);
		case 6:
			disk_name = string(argv[1]);
			op1 = string(argv[2]);
			op2 = string(argv[3]);
			op3 = string(argv[4]);
			op4 = string(argv[5]);
		default:
			printf("USAGE: ./ssfs <disk file name> <op1> <op2> <op3>\n");
			return 0;		
	}
	int threads = argc - 2;

	Command c1;
	c1.command = "WRITE";
	c1.character = 'd';
	c1.start_byte = 1;
	c1.num_bytes = 3;

	Command c2;
	c2.command = "WRITE";
	c2.character = 'f';
	c2.start_byte = 0;
	c2.num_bytes = 10;

	thread scheduler = thread(execute_commands, 0);

	command_mutex.lock();
	commands.push_back(c1);
	commands.push_back(c2);
	command_mutex.unlock();

	scheduler.join();
}















