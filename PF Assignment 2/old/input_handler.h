#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <queue>
#include <mutex>

using namespace std;

class input_handler
{
private:
	queue<unsigned char> input_queue;

	mutex queue_lock;

	void loop_main();

public:
	void start();

	unsigned char dequeue();
};

#endif