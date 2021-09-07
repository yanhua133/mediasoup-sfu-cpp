#include "timer.hpp"
#include <random>

using std::default_random_engine;
using std::uniform_real_distribution;


ChannelTimer::ChannelTimer() {

}
ChannelTimer::~ChannelTimer() {

}

void ChannelTimer::setTimeout(std::function<void()> const& func, int delay) {
	this->clear = false;
	std::thread t([=]() {
		if (this->clear) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		if (this->clear) return;
		func();
		});
	t.detach();
}

void ChannelTimer::setInterval(std::function<void()> const& func, int interval) {
	this->clear = false;
	std::thread t([=]() {
		while (true) {
			if (this->clear) return;
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			if (this->clear) return;
			func();
		}
		});
	t.detach();
}

void ChannelTimer::stop() {
	this->clear = true;
}
