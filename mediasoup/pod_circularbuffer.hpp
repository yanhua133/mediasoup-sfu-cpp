/***************************************
* @file     pod_circularbuffer.h
* @brief    环形缓冲区-支持存储pod数据类型，直接用memcpy连续复制多个数据
* @details  出处：http://blog.163.com/qimo601@126/blog/static/1582209320121169244219/
            原作者地址：http://blog.csdn.net/devday/article/details/5258697
* @author   phata, wqvbjhc@gmail.com
* @date     2014-5-20
****************************************/
#ifndef POD_CIRCULAR_BUFFER_H
#define POD_CIRCULAR_BUFFER_H
#include <assert.h>
#include <memory.h>
template<typename T>
class PodCircularBuffer
{
public:
    PodCircularBuffer(int capacity)
        :m_nBufSize(capacity),m_nReadPos(0),m_nWritePos(0)
        ,m_bEmpty(true),m_bFull(false) {
        m_pBuf = new T[m_nBufSize];
    }
    virtual ~PodCircularBuffer() {
        delete[] m_pBuf;
    }

    bool full() const{
        return m_bFull;
    }
    bool empty() const{
        return m_bEmpty;
    }
    void clear() {
        m_nReadPos = 0;
        m_nWritePos = 0;
        m_bEmpty = true;
        m_bFull = false;
    }

    /************************************************************************/
    /* 获取缓冲区有效数据长度                                               */
    /************************************************************************/
    size_t size() const {
        if(m_bEmpty) {
            return 0;
        } else if(m_bFull) {
            return m_nBufSize;
        } else if(m_nReadPos < m_nWritePos) {
            return m_nWritePos - m_nReadPos;
        } else {
            return m_nBufSize - m_nReadPos + m_nWritePos;
        }
    }

    size_t capacity() const {
        return m_nBufSize;
    }
	
	T* data() {
		return m_pBuf;
	}

	const T* data() const{
		return m_pBuf;
	}
    /************************************************************************/
    /* 向缓冲区写入数据，返回实际写入的对象数                               */
    /************************************************************************/
    int write(const T* buf, int count) {
        if(count <= 0) {
            return 0;
        }
        if(m_bFull) {// 缓冲区已满，不能继续写入
            return 0;
        }
        m_bEmpty = false;
        if(m_nReadPos == m_nWritePos) { // 缓冲区为空时
            /*                          == 内存模型 ==
               (empty)             m_nReadPos                (empty)
            |----------------------------------|-----------------------------------------|
                   m_nWritePos        m_nBufSize
            */
            int leftcount = m_nBufSize - m_nWritePos;
            if(leftcount > count) {
                memcpy(&m_pBuf[m_nWritePos], buf, count * sizeof(T));
                m_nWritePos += count;
                return count;
            } else {
                memcpy(&m_pBuf[m_nWritePos], buf, leftcount * sizeof(T));
                m_nWritePos = (m_nReadPos > count - leftcount) ? count - leftcount : m_nWritePos;
                memcpy(m_pBuf, &buf[leftcount], m_nWritePos * sizeof(T));
                m_bFull = (m_nWritePos == m_nReadPos);
                return leftcount + m_nWritePos;
            }
        } else if(m_nReadPos < m_nWritePos) { // 有剩余空间可写入
            /*                           == 内存模型 ==
             (empty)                 (data)                     (empty)
            |-------------------|----------------------------|---------------------------|
               m_nReadPos                m_nWritePos       (leftcount)
            */
            // 剩余缓冲区大小(从写入位置到缓冲区尾)

            int leftcount = m_nBufSize - m_nWritePos;
            if(leftcount > count) { // 有足够的剩余空间存放
                memcpy(&m_pBuf[m_nWritePos], buf, count * sizeof(T));
                m_nWritePos += count;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            } else {   // 剩余空间不足
                // 先填充满剩余空间，再回头找空间存放
                memcpy(&m_pBuf[m_nWritePos], buf, leftcount * sizeof(T));
                m_nWritePos = (m_nReadPos >= count - leftcount) ? count - leftcount : m_nReadPos;
                memcpy(m_pBuf, &buf[leftcount], m_nWritePos * sizeof(T));
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount + m_nWritePos;
            }
        } else {
            /*                          == 内存模型 ==
             (unread)                 (read)                     (unread)
            |-------------------|----------------------------|---------------------------|
                m_nWritePos    (leftcount)    m_nReadPos
            */
            int leftcount = m_nReadPos - m_nWritePos;
            if(leftcount > count) {
                // 有足够的剩余空间存放
                memcpy(&m_pBuf[m_nWritePos], buf, count * sizeof(T));
                m_nWritePos += count;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            } else {
                // 剩余空间不足时要丢弃后面的数据
                memcpy(&m_pBuf[m_nWritePos], buf, leftcount * sizeof(T));
                m_nWritePos += leftcount;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_bFull);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount;
            }
        }
    }

    /************************************************************************/
    /* 从缓冲区读数据，返回实际读取的字节数                                 */
    /************************************************************************/
    int read(T* buf, int count) {
        if(count <= 0) {
            return 0;
        }
        if(m_bEmpty) {// 缓冲区空，不能继续读取数据
            return 0;
        }
        m_bFull = false;
        if(m_nReadPos == m_nWritePos) { // 缓冲区满时
            /*                          == 内存模型 ==
             (data)          m_nReadPos                (data)
            |--------------------------------|--------------------------------------------|
              m_nWritePos         m_nBufSize
            */
            int leftcount = m_nBufSize - m_nReadPos;
            if(leftcount > count) {
                memcpy(buf, &m_pBuf[m_nReadPos], count * sizeof(T));
                m_nReadPos += count;
                m_bEmpty = (m_nReadPos == m_nWritePos);
                return count;
            } else {
                memcpy(buf, &m_pBuf[m_nReadPos], leftcount * sizeof(T));
                m_nReadPos = (m_nWritePos > count - leftcount) ? count - leftcount : m_nWritePos;
                memcpy(&buf[leftcount], m_pBuf, m_nReadPos * sizeof(T));
                m_bEmpty = (m_nReadPos == m_nWritePos);
                return leftcount + m_nReadPos;
            }
        } else if(m_nReadPos < m_nWritePos) { // 写指针在前(未读数据是连接的)
            /*                          == 内存模型 ==
             (read)                 (unread)                      (read)
            |-------------------|----------------------------|---------------------------|
               m_nReadPos                m_nWritePos                     m_nBufSize
            */
            int leftcount = m_nWritePos - m_nReadPos;
            int c = (leftcount > count) ? count : leftcount;
            memcpy(buf, &m_pBuf[m_nReadPos], c * sizeof(T));
            m_nReadPos += c;
            m_bEmpty = (m_nReadPos == m_nWritePos);
            assert(m_nReadPos <= m_nBufSize);
            assert(m_nWritePos <= m_nBufSize);
            return c;
        } else {      // 读指针在前(未读数据可能是不连接的)
            /*                          == 内存模型 ==
               (unread)                (read)                      (unread)
            |-------------------|----------------------------|---------------------------|
                m_nWritePos                  m_nReadPos                  m_nBufSize

            */
            int leftcount = m_nBufSize - m_nReadPos;
            if(leftcount > count) { // 未读缓冲区够大，直接读取数据
                memcpy(buf, &m_pBuf[m_nReadPos], count * sizeof(T));
                m_nReadPos += count;
                m_bEmpty = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            } else {   // 未读缓冲区不足，需回到缓冲区头开始读
                memcpy(buf, &m_pBuf[m_nReadPos], leftcount * sizeof(T));
                m_nReadPos = (m_nWritePos >= count - leftcount) ? count - leftcount : m_nWritePos;
                memcpy(&buf[leftcount], m_pBuf, m_nReadPos * sizeof(T));
                m_bEmpty = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount + m_nReadPos;
            }
        }
    }

private:
    bool m_bEmpty, m_bFull;
    T * m_pBuf;
    int m_nBufSize;
    int m_nReadPos;
    int m_nWritePos;
private://Noncopyable
    PodCircularBuffer(const PodCircularBuffer&);
    const PodCircularBuffer& operator=(const PodCircularBuffer&);
};
#endif // POD_CIRCULAR_BUFFER_H

////测试用例
//#include <stdio.h>
//#include <iostream>
//#include <stdint.h>
//#include <assert.h>
//#include "pod_circularbuffer.h"
//int main(int argc, char **argv)
//{
//	{
//		PodCircularBuffer<char> objbuf(1000);
//		char tmpwchar[10]= {'1','2'};
//		char tmprchar[15];
//		assert(10 == objbuf.write(tmpwchar,sizeof(tmpwchar)));
//		objbuf.read(tmprchar,2);
//		assert(tmprchar[0] == '1');
//		assert(tmprchar[1] == '2');
//		int count = 0;
//		while(++count < 10) {
//			for (int i=0; i< 11; ++i) {
//				int iret = objbuf.write(tmpwchar,sizeof(tmpwchar));
//				printf("write %d, size %d, capacity %d\n",iret,objbuf.size(),objbuf.capacity());
//			}
//			for (int i=0; i< 7; ++i) {
//				int iret = objbuf.read(tmprchar,sizeof(tmprchar));
//				printf("read %d, size %d, capacity %d\n",iret,objbuf.size(),objbuf.capacity());
//			}
//		}
//	}
//	{
//		typedef struct strdata{
//			int idata;
//			float fdata;
//			double ddata;
//			char cdata[7];
//		}strdata;
//#define  ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
//		printf("\n\n\nstart opt struct\n");
//		PodCircularBuffer<strdata> objbuf(1000);
//		strdata tmpwchar[10];
//		strdata tmprchar[15];
//		assert(10 == objbuf.write(tmpwchar,ARRAYSIZE(tmpwchar)));
//		objbuf.read(tmprchar,2);
//		int count = 0;
//		while(++count < 10) {
//			for (int i=0; i< 11; ++i) {
//				int iret = objbuf.write(tmpwchar,ARRAYSIZE(tmpwchar));
//				printf("write %d, size %d, capacity %d\n",iret,objbuf.size(),objbuf.capacity());
//			}
//			for (int i=0; i< 7; ++i) {
//				int iret = objbuf.read(tmprchar,ARRAYSIZE(tmprchar));
//				printf("read %d, size %d, capacity %d\n",iret,objbuf.size(),objbuf.capacity());
//			}
//		}
//	}
//	return 0;
//}
