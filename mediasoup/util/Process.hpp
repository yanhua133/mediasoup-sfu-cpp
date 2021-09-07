#pragma once
#include "../Singleton.hpp"
#include "uv.h"


#include <string>
#include <map>

namespace mediasoup
{

class Process : public Singleton<Process> {
public:
    Process();
    virtual ~Process();

	const std::string& Environ(const std::string& name);
	const std::string& ExePath();

private:
	uv_env_item_t* m_envItem = nullptr;
	int m_envCount = 0;
	std::map<std::string, std::string> m_mapEnvs;
	std::string m_emptyEnv = "";
	std::string m_exePath = "";
};

}
