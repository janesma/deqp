/*-------------------------------------------------------------------------
 * drawElements Quality Program Execution Server
 * ---------------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Execution Server Protocol.
 *//*--------------------------------------------------------------------*/

#include "xsProtocol.hpp"

using std::string;
using std::vector;

namespace xs
{

inline deUint32 swapEndianess (deUint32 value)
{
	deUint32 b0 = (value >>  0) & 0xFF;
	deUint32 b1 = (value >>  8) & 0xFF;
	deUint32 b2 = (value >> 16) & 0xFF;
	deUint32 b3 = (value >> 24) & 0xFF;
	return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

template <typename T> T networkToHost (T value);
template <typename T> T hostToNetwork (T value);

template <> int networkToHost (int value) { return (int)swapEndianess((deUint32)value); }
template <> int hostToNetwork (int value) { return (int)swapEndianess((deUint32)value); }

class MessageParser
{
public:
	MessageParser (const deUint8* data, int dataSize)
		: m_data	(data)
		, m_size	(dataSize)
		, m_pos		(0)
	{
	}

	template <typename T>
	T get (void)
	{
		XS_CHECK_MSG(m_pos + (int)sizeof(T) <= m_size, "Invalid payload size");
		T netValue;
		deMemcpy(&netValue, &m_data[m_pos], sizeof(T));
		m_pos += sizeof(T);
		return networkToHost(netValue);
	}

	void getString (std::string& dst)
	{
		// \todo [2011-09-30 pyry] We should really send a size parameter instead.
		while (m_data[m_pos] != 0)
		{
			dst += (char)m_data[m_pos++];
			XS_CHECK_MSG(m_pos < m_size, "Unterminated string payload");
		}

		m_pos += 1;
	}

	void assumEnd (void)
	{
		if (m_pos != m_size)
			XS_FAIL("Invalid payload size");
	}

private:
	const deUint8*	m_data;
	int				m_size;
	int				m_pos;
};

class MessageWriter
{
public:
	MessageWriter (MessageType msgType, std::vector<deUint8>& buf)
		: m_buf(buf)
	{
		// Place for size.
		put<int>(0);

		// Write message type.
		put<int>(msgType);
	}

	~MessageWriter (void)
	{
		finalize();
	}

	void finalize (void)
	{
		DE_ASSERT(m_buf.size() >= MESSAGE_HEADER_SIZE);

		// Write actual size.
		int size = hostToNetwork((int)m_buf.size());
		deMemcpy(&m_buf[0], &size, sizeof(int));
	}

	template <typename T>
	void put (T value)
	{
		T netValue = hostToNetwork(value);
		size_t curPos = m_buf.size();
		m_buf.resize(curPos + sizeof(T));
		deMemcpy(&m_buf[curPos], &netValue, sizeof(T));
	}

private:
	std::vector<deUint8>& m_buf;
};

template <>
void MessageWriter::put<const char*> (const char* value)
{
	int curPos = (int)m_buf.size();
	int strLen = (int)strlen(value);

	m_buf.resize(curPos + strLen+1);
	deMemcpy(&m_buf[curPos], &value[0], strLen+1);
}

void Message::parseHeader (const deUint8* data, int dataSize, MessageType& type, int& size)
{
	XS_CHECK_MSG(dataSize >= MESSAGE_HEADER_SIZE, "Incomplete header");
	MessageParser parser(data, dataSize);
	size	= (MessageType)parser.get<int>();
	type	= (MessageType)parser.get<int>();
}

void Message::writeHeader (MessageType type, int messageSize, deUint8* dst, int bufSize)
{
	XS_CHECK_MSG(bufSize >= MESSAGE_HEADER_SIZE, "Incomplete header");
	int netSize = hostToNetwork(messageSize);
	int netType = hostToNetwork((int)type);
	deMemcpy(dst+0, &netSize, sizeof(netSize));
	deMemcpy(dst+4, &netType, sizeof(netType));
}

void Message::writeNoData (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
}

HelloMessage::HelloMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_HELLO)
{
	MessageParser parser(data, dataSize);
	version = parser.get<int>();
	parser.assumEnd();
}

void HelloMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(version);
}

TestMessage::TestMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_TEST)
{
	MessageParser parser(data, dataSize);
	parser.getString(test);
	parser.assumEnd();
}

void TestMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(test.c_str());
}

ExecuteBinaryMessage::ExecuteBinaryMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_EXECUTE_BINARY)
{
	MessageParser parser(data, dataSize);
	parser.getString(name);
	parser.getString(params);
	parser.getString(workDir);
	parser.getString(caseList);
	parser.assumEnd();
}

void ExecuteBinaryMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(name.c_str());
	writer.put(params.c_str());
	writer.put(workDir.c_str());
	writer.put(caseList.c_str());
}

ProcessLogDataMessage::ProcessLogDataMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_PROCESS_LOG_DATA)
{
	MessageParser parser(data, dataSize);
	parser.getString(logData);
	parser.assumEnd();
}

void ProcessLogDataMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(logData.c_str());
}

ProcessLaunchFailedMessage::ProcessLaunchFailedMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_PROCESS_LAUNCH_FAILED)
{
	MessageParser parser(data, dataSize);
	parser.getString(reason);
	parser.assumEnd();
}

void ProcessLaunchFailedMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(reason.c_str());
}

ProcessFinishedMessage::ProcessFinishedMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_PROCESS_FINISHED)
{
	MessageParser parser(data, dataSize);
	exitCode = parser.get<int>();
	parser.assumEnd();
}

void ProcessFinishedMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(exitCode);
}

InfoMessage::InfoMessage (const deUint8* data, int dataSize)
	: Message(MESSAGETYPE_INFO)
{
	MessageParser parser(data, dataSize);
	parser.getString(info);
	parser.assumEnd();
}

void InfoMessage::write (vector<deUint8>& buf) const
{
	MessageWriter writer(type, buf);
	writer.put(info.c_str());
}

} // xs
