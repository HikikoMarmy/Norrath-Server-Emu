
#include "../global_define.h"

RealmSocket::RealmSocket()
{
	fd = INVALID_SOCKET;

	memset( &local_address, 0, sizeof( local_address ) );
	memset( &remote_address, 0, sizeof( remote_address ) );
	port = 0;

	flag.disconnected = 0;
	flag.is_listener = 0;
	flag.want_more_read_data = 0;
	flag.want_more_write_data = 0;

	last_write_position = 0;

	latency = 0;
	last_recv_time = 0;
	last_send_time = 0;
}

RealmSocket::~RealmSocket()
{
	if( INVALID_SOCKET != fd )
	{
		closesocket( fd );
	}

	fd = INVALID_SOCKET;

	memset( &local_address, 0, sizeof( local_address ) );
	memset( &remote_address, 0, sizeof( remote_address ) );
	port = 0;

	flag.disconnected = 0;
	flag.is_listener = 0;
	flag.want_more_read_data = 0;
	flag.want_more_write_data = 0;

	last_write_position = 0;

	latency = 0;
	last_recv_time = 0;
	last_send_time = 0;
}

RealmTCPSocket::RealmTCPSocket()
{
	m_pendingWriteBuffer.reserve( WRITE_BUFFER_SIZE );
}

RealmTCPSocket::~RealmTCPSocket()
{
	
}

void RealmTCPSocket::send( const sptr_byte_stream stream )
{
	auto packetSize = htonl( stream->get_position() );

	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), ( uint8_t * )&packetSize, ( uint8_t * )&packetSize + 4 );
	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), stream->data.begin(), stream->data.end() );
}

void RealmTCPSocket::send( const ByteStream &stream )
{
	auto packetSize = htonl( stream.get_position() );

	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), ( uint8_t * )&packetSize, ( uint8_t * )&packetSize + 4 );
	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), stream.data.begin(), stream.data.end() );
}

void RealmTCPSocket::send( const sptr_generic_response response )
{
	auto &stream = response->Serialize();
	auto netSize = htonl( stream.get_position() + 4 );

	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), ( uint8_t * )&netSize, ( uint8_t * )&netSize + 4 );
	m_pendingWriteBuffer.insert( m_pendingWriteBuffer.end(), stream.data.begin(), stream.data.end() );

	//Log::Packet( stream->data, packetSize, true );
}

RealmUDPSocket::RealmUDPSocket()
{
}

RealmUDPSocket::~RealmUDPSocket()
{
}

void RealmUDPSocket::send( const sptr_byte_stream stream )
{
	m_pendingWriteQueue.push( stream );
}

void RealmUDPSocket::send( const ByteStream &stream )
{
	m_pendingWriteQueue.push( std::make_shared< ByteStream >( stream ) );
}