#pragma once

#include "uv.h"
#include "../common/CommonObserver.hpp"

namespace mediasoup
{

class UVPipeWrapper;
class UVPipeObserver {
public:
	virtual void OnRead(UVPipeWrapper* pipe, uint8_t* data, size_t len) = 0;
	virtual void OnClose(UVPipeWrapper* uvHandle) = 0;
};

class UVPipeWrapper : public CommonObserver<UVPipeObserver> {
public:
	enum class Role {
		PRODUCER = 1,
		CONSUMER
	};	

	/* Struct for the data field of uv_req_t when writing data. */
	struct UvWriteData {
		explicit UvWriteData(size_t storeSize) {
			this->store = new uint8_t[storeSize];
		}

		// Disable copy constructor because of the dynamically allocated data (store).
		UvWriteData(const UvWriteData&) = delete;

		~UvWriteData() {
			delete[] this->store;
		}

		uv_write_t req;
		uint8_t* store = nullptr;
	};

    UVPipeWrapper(UVPipeObserver* obs, int bufferSize, UVPipeWrapper::Role role);
    virtual ~UVPipeWrapper();

	bool Init(int fd);
	bool Start();
	uv_pipe_t* GetPipe() const;
	void Write(const uint8_t* data, size_t len);
	void Close();

public:
	void OnAllocCB(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);
	void OnReadCB(uv_stream_t* stream, ssize_t nRead, const uv_buf_t* buf);
	void OnWriteError(int error);

public:
	uv_mutex_t m_mutex_writebuf;
	UvWriteData* m_writeData;
	char* m_inputDataBuf; 
	int  m_inputLen; 
private:
	uv_pipe_t* m_uvHandle = nullptr;

	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_bufferDataLen = 0;

	bool m_closed = false;
	bool m_isClosedByPeer = false;
	bool m_hasError = false;
  int m_fd = 0;

	UVPipeWrapper::Role m_role = Role::CONSUMER;
};

}