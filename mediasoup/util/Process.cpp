
#include "Process.hpp"
#include "../Log.hpp"

namespace mediasoup
{

Process::Process() {
	MS_lOGF();
	int ret = uv_os_environ(&m_envItem, &m_envCount);
	MS_ASSERT_R_LOGE(0 == ret, "uv_os_environ failed:{}", ret);

	for (int i = 0; i < m_envCount; i++) {
		m_mapEnvs[m_envItem[i].name] = m_envItem[i].value;
	}
}

Process::~Process() {
	MS_lOGF();
	if (m_envItem) {
		uv_os_free_environ(m_envItem, m_envCount);
		m_envItem = nullptr;
		m_envCount = 0;
	}
	m_mapEnvs.clear();
}

const std::string& Process::Environ(const std::string& name) {
	MS_lOGF();
	if (m_mapEnvs.find(name) == m_mapEnvs.end()) {
		return m_emptyEnv;
	}
    
	return m_mapEnvs[name];
}

const std::string& Process::ExePath() {
	MS_lOGF();
	if (m_exePath.empty()) {
		char exePath[255] = { 0 };
		size_t exePathLen = 255;
		uv_exepath(exePath, &exePathLen);
		m_exePath = exePath;
	}

	return m_exePath;
}

}
