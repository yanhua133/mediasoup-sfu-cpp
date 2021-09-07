#pragma once
#include <functional>

namespace mediasoup
{

class AutoRelease {
public:
    AutoRelease(std::function<void()> release) 
		: m_release(release) {

    }

    ~AutoRelease() {
		if (m_release) {
			m_release();
		}
    }
private:
	std::function<void()> m_release = nullptr;
};

}