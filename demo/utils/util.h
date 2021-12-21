#ifndef PEERTIMER_H
#define PEERTIMER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
//生成随机数(0-1)*10000000
int generateRandomNumber();


class PeerTimer {
	bool clear = false;
public:
	PeerTimer();
	~PeerTimer();

public:
	void setTimeout(std::function<void()> const& func, int delay);
	void setInterval(std::function<void()> const& func, int interval);
	void stop();

};

#endif
