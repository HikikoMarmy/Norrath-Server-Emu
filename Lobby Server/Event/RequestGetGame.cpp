#include "../../global_define.h"
#include "RequestGetGame.h"

void RequestGetGame::Deserialize( sptr_byte_stream stream )
{
	DeserializeHeader( stream );

	m_sessionId = stream->read_encrypted_utf16();
	m_gameName = stream->read_utf16();
}

sptr_generic_response RequestGetGame::ProcessRequest( sptr_user user, sptr_byte_stream stream )
{
	Deserialize( stream );

	if( user == nullptr )
	{
		Log::Error( "User not found! [%S]", m_sessionId.c_str() );
		return std::make_shared< ResultGetGame >( this, TIMEOUT );
	}

	auto session = GameSessionManager::Get().FindGame( m_gameName );

	if( session == nullptr )
	{
		Log::Error( "Game session not found! [%S]", m_gameName.c_str() );
		return std::make_shared< ResultGetGame >( this, NOT_FOUND );
	}

	if( session->m_currentPlayers >= session->m_maximumPlayers )
	{
		Log::Error( "Game session is full! [%S]", m_gameName.c_str() );
		return std::make_shared< ResultGetGame >( this, TIMEOUT );
	}

	auto host_user = session->m_owner.lock();

	if( host_user == nullptr )
	{
		Log::Error( "Game session owner not found! [%S]", m_gameName.c_str() );
		return std::make_shared< ResultGetGame >( this, TIMEOUT );
	}

	user->m_isHost = false;
	user->m_gameId = session->m_gameIndex;

	return std::make_shared< ResultGetGame >( this, SUCCESS, session->m_gameIndex );
}

ResultGetGame::ResultGetGame( GenericRequest *request, int32_t reply, int32_t gameId ) : GenericResponse( *request )
{
	m_reply = reply;
	m_gameId = gameId;
}

ByteStream &ResultGetGame::Serialize()
{
	m_stream.write_u16( m_packetId );
	m_stream.write_u32( m_requestId );
	m_stream.write_u32( m_reply );

	// TODO: These may come in from the UpdateGameData event
	m_stream.write_utf16( L"Kelethin" );
	m_stream.write_utf16( L"OwnerName" );

	m_stream.write_u32( m_gameId );

	return m_stream;
}
