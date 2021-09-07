#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>



class ChannelTimer {
	bool clear = false;
public:
    ChannelTimer();
	~ChannelTimer();

public:
	void setTimeout(std::function<void()> const& func, int delay);
	void setInterval(std::function<void()> const& func, int interval);
	void stop();

};

#endif
