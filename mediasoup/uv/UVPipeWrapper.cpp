#include "UVPipeWrapper.hpp"
#include "../Log.hpp"
#include "../Mediasoup.hpp"

namespace mediasoup
{

inline static void StaticOnAllocCB(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) {
	((UVPipeWrapper*)(handle->data))->OnAllocCB(handle, suggestedSize, buf);
}

inline static void StaticOnReadCB(uv_stream_t* stream, ssize_t nRead, const uv_buf_t* buf) {
	UVPipeWrapper* pWrapper = (UVPipeWrapper*)stream->data;
	uv_mutex_lock(&pWrapper->m_mutex_writebuf); 

	((UVPipeWrapper*)(stream->data))->OnReadCB(stream, nRead, buf);
}

inline static void StaticOnClose(uv_handle_t* handle) {
	MS_lOGF();
	delete handle;
}

inline static void StaticcOnShutdown(uv_shutdown_t* req, int /*status*/) {
	auto* handle = req->handle;

	delete req;

	// Now do close the handle.
	uv_close(reinterpret_cast<uv_handle_t*>(handle), static_cast<uv_close_cb>(StaticOnClose));
}

inline static void StaticcOnWrite(uv_write_t* req, int status)
{
	UVPipeWrapper * pUvPipeWrapper = static_cast<UVPipeWrapper *>(req->data);
	auto* writeData = pUvPipeWrapper->m_writeData;
	auto* handle = req->handle;
	auto* socket = static_cast<UVPipeWrapper*>(handle->data);

	// Just notify the UnixStreamSocket when error.
	if (socket && status != 0)
		socket->OnWriteError(status);

	// Delete the UvWriteData struct.

	delete writeData;
	pUvPipeWrapper->m_writeData = NULL;
	uv_mutex_unlock(&pUvPipeWrapper->m_mutex_writebuf);
}

UVPipeWrapper::UVPipeWrapper(UVPipeObserver* obs, int bufferSize, UVPipeWrapper::Role role)
	: m_bufferSize(bufferSize)
	, m_role(role)
{
    MS_lOGF();
	RegisterObserver(obs);

	m_inputDataBuf = nullptr;
	m_inputLen = 0;
	m_writeData = nullptr;
	int iret = uv_mutex_init(&m_mutex_writebuf);
	if (iret) {
		MS_lOGE("UVPipeWrapper () uv mutex init failed: %s", uv_strerror(iret));
	}


}

UVPipeWrapper::~UVPipeWrapper() {
    MS_lOGF();
	if (!m_closed) {
		Close();
	}

	uv_mutex_destroy(&m_mutex_writebuf);
		
	if (m_buffer) {
		delete[] m_buffer;
		m_buffer = nullptr;
	}

	if (m_writeData) {
		delete[] m_writeData;
		m_writeData = nullptr;
	}

	if (m_inputDataBuf) {
		delete[] m_inputDataBuf;
		m_inputLen = 0;
	}
}

bool UVPipeWrapper::Init(int fd) {
	MS_lOGF();
	MS_ASSERT_RV_LOGI(!m_uvHandle, true, "already init");

  m_fd = fd ;
	m_uvHandle = new uv_pipe_t;
	m_uvHandle->data = static_cast<void*>(this);

	if (m_inputDataBuf) {
		delete[] m_inputDataBuf;
		m_inputLen = 0;
	}
	m_inputLen = 1024 * 100;
	m_inputDataBuf = new char[m_inputLen];


	// ipc: whether this pipeline passes handles between different processes
	int ret = uv_pipe_init(Mediasoup::GetInstance().GetLoop(), m_uvHandle, 0);

	if (0 != ret) {
		delete m_uvHandle;
		m_uvHandle = nullptr;

		MS_lOGE("uv_pipe_init() failed: %s", uv_strerror(ret));
		return false;
	}

  ret  = uv_pipe_open(m_uvHandle, fd);

	if (ret != 0)
	{
		delete m_uvHandle;
		m_uvHandle = nullptr;

		MS_lOGE("uv_pipe_open() failed: %s", uv_strerror(ret));
		return false;
	}
    MS_lOGD("uv_pipe_open() success: m_fd=%d m_role=%d", m_fd,m_role);
	return true;
}

bool UVPipeWrapper::Start() {
	MS_lOGF();
	MS_ASSERT_RV_LOGE(m_uvHandle, false, "no init");
   
	if (UVPipeWrapper::Role::CONSUMER == m_role) {
		// Start reading.
        MS_lOGD("UVPipeWrapper::Start(): fd=%d m_role=CONSUMER", m_fd);
		int ret = uv_read_start(reinterpret_cast<uv_stream_t*>(m_uvHandle),
			static_cast<uv_alloc_cb>(StaticOnAllocCB),
			static_cast<uv_read_cb>(StaticOnReadCB));

		MS_ASSERT_RV_LOGE(0 == ret, false, "uv_read_start() failed: %s fd=%d", uv_strerror(ret),m_fd);
	}

	return true;
}

uv_pipe_t* UVPipeWrapper::GetPipe() const {
	MS_lOGF();

	return m_uvHandle;
}

void UVPipeWrapper::Write(const uint8_t* data, size_t len) {
	MS_lOGF();

	Mediasoup::GetInstance().Send((const char*)data, len, m_uvHandle);
}
/*
void UVPipeWrapper::Write(const uint8_t* data, size_t len) {
	MS_lOGF();
	MS_ASSERT_R_LOGI(m_uvHandle, "no init");
	MS_ASSERT_R_LOGE(!m_closed, "pipi closed already");
	MS_ASSERT_R_LOGE(len != 0, "len == 0");	

	uv_mutex_lock(&m_mutex_writebuf); //add by jacky 20211018
	// First try uv_try_write(). In case it can not directly send all the given data
	// then build a uv_req_t and use uv_write().

	if (m_writeData != nullptr) {
		fprintf(stderr, "m_writeData != nullptr ,why?\n");//add by jacky 20211018
	}
	if (m_inputLen < len) {
		if ((m_inputLen > 0) && (m_inputDataBuf != nullptr)) {
			delete[] m_inputDataBuf;
			m_inputDataBuf = nullptr;
		}
		m_inputLen = len ;

		m_inputDataBuf = new char[m_inputLen];
	}
	memcpy(m_inputDataBuf, data, len);
	uv_buf_t buffer = uv_buf_init(m_inputDataBuf, len); //add by jacky 20211019
	m_writeData = new UvWriteData(len);

	m_writeData->req.data = this;// static_cast<void*>(m_writeData);

	int err = uv_write(
		&m_writeData->req,
		reinterpret_cast<uv_stream_t*>(m_uvHandle),
		&buffer,
		1,
		static_cast<uv_write_cb>(StaticcOnWrite));

	if (err != 0) {
		MS_lOGE("uv_write() failed: %s", uv_strerror(err));		

		uv_mutex_unlock(&m_mutex_writebuf);
		// Delete the UvSendData struct.
		delete m_writeData;
		m_writeData = nullptr;
	}
}
*/
void UVPipeWrapper::Close() {
	MS_lOGF();
	MS_ASSERT_R_LOGI(m_uvHandle, "no init");
	MS_ASSERT_R_LOGI(!m_closed, "pipe closed already");
	
	m_closed = true;

	// Tell the UV handle that the UnixStreamSocket has been closed.
	m_uvHandle->data = nullptr;

	int ret = -1;
	if (m_role == UVPipeWrapper::Role::CONSUMER) {
		// Don't read more.
		ret = uv_read_stop(reinterpret_cast<uv_stream_t*>(m_uvHandle));

		MS_ASSERT_R_LOGE(ret == 0, "uv_read_stop() failed: %s", uv_strerror(ret));
	}

	// If there is no error and the peer didn't close its pipe side then close gracefully.
	// todo: pipe need shutdown?
	if (0 && !m_hasError && !m_isClosedByPeer) {
		// Use uv_shutdown() so pending data to be written will be sent to the peer before closing.
		auto req = new uv_shutdown_t;
		req->data = static_cast<void*>(this);
		ret = uv_shutdown(req, reinterpret_cast<uv_stream_t*>(m_uvHandle),
			static_cast<uv_shutdown_cb>(StaticcOnShutdown));

		MS_ASSERT_R_LOGE(ret == 0, "uv_shutdown() failed: %s", uv_strerror(ret));
	} else {
		// Otherwise directly close the socket.
		uv_close(reinterpret_cast<uv_handle_t*>(m_uvHandle), static_cast<uv_close_cb>(StaticOnClose));
	}
	if (m_writeData != nullptr) { 
		uv_mutex_unlock(&m_mutex_writebuf);
		// Delete the UvSendData struct.
		delete m_writeData;
		m_writeData = nullptr;
	}
}

void UVPipeWrapper::OnAllocCB(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) {
	// If this is the first call to onUvReadAlloc() then allocate the receiving buffer now.
	if (m_buffer == nullptr) {
		m_buffer = new uint8_t[m_bufferSize];
	}	

	// Tell UV to write after the last data byte in the buffer.
	buf->base = reinterpret_cast<char*>(m_buffer + m_bufferDataLen);

	// Give UV all the remaining space in the buffer.
	if (m_bufferSize > m_bufferDataLen) {
		buf->len = m_bufferSize - m_bufferDataLen;
	} else {
		buf->len = 0;

		MS_lOGE("no available space in the buffer");
	}
}

void UVPipeWrapper::OnReadCB(uv_stream_t* stream, ssize_t nRead, const uv_buf_t* buf) {
	if (nRead == 0) {
		return;
	}
	
	// Data received.
	if (nRead > 0) {
		// Update the buffer data length.
		m_bufferDataLen += static_cast<size_t>(nRead);

		// Notify the Observer.
		NotifyObserver(std::bind(&UVPipeObserver::OnRead, std::placeholders::_1, this, m_buffer, m_bufferDataLen));		
		m_bufferDataLen = 0;
	} else if (nRead == UV_EOF || nRead == UV_ECONNRESET) {
		// Peer disconnected.	
		m_isClosedByPeer = true;

		// Close local side of the pipe.
		Close();

		// Notify the Observer.
		NotifyObserver(std::bind(&UVPipeObserver::OnClose, std::placeholders::_1, this));
	} else {
		// Some error.
		MS_lOGE("read error, closing the pipe: %s", uv_strerror(nRead));

		m_hasError = true;

		// Close the socket.
		Close();

		// Notify the Observer.
		NotifyObserver(std::bind(&UVPipeObserver::OnClose, std::placeholders::_1, this));
	}
}

void UVPipeWrapper::OnWriteError(int error) {
	MS_lOGF();

	if (error != UV_EPIPE && error != UV_ENOTCONN) {
		m_hasError = true;
	}		

	MS_lOGE("write error, closing the pipe: %s", uv_strerror(error));

	Close();

	// Notify the Observer.
	NotifyObserver(std::bind(&UVPipeObserver::OnClose, std::placeholders::_1, this));
}

}
