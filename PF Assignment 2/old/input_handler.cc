#include "input_handler.h"

#include <thread>
#include <WinUser.h>

void input_handler::loop_main()
{
	queue_lock.lock();
	// TODO: we may not need this
}

void input_handler::start()
{
	thread thrd(&input_handler::loop_main, this);
}

unsigned char input_handler::dequeue()
{
	queue_lock.lock();
	unsigned char value;
	if (input_queue.empty())
		value = 0;
	else
	{
		value = input_queue.front();
		input_queue.pop();
	}
	queue_lock.unlock();
	return value;
}
