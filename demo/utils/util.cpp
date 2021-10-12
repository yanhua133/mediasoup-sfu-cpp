#include "util.h"
#include <random>

using std::default_random_engine;
using std::uniform_real_distribution;

int generateRandomNumber() {
	default_random_engine e;
	uniform_real_distribution<double> u(0, 1);
	return int(u(e) * 10000000);
}

PeerTimer::PeerTimer() {

}
PeerTimer::~PeerTimer() {

}

void PeerTimer::setTimeout(std::function<void()> const& func, int delay) {
	this->clear = false;
	std::thread t([=]() {
		if (this->clear) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		if (this->clear) return;
		func();
		});
	t.detach();
}

void PeerTimer::setInterval(std::function<void()> const& func, int interval) {
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

void PeerTimer::stop() {
	this->clear = true;
}
