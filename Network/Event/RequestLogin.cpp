#include "../../global_define.h"
#include "RequestLogin.h"

void RequestLogin::Deserialize( sptr_byte_stream stream )
{
	DeserializeHeader( stream );

	m_username = stream->read_encrypted_utf16();
	m_password = stream->read_encrypted_utf16();
}

sptr_generic_response RequestLogin::ProcessRequest( sptr_user user, sptr_byte_stream stream )
{
	Deserialize( stream );

	Log::Packet( stream->data, stream->data.size(), false );

	if( m_username.empty() || m_password.empty() )
	{
		Log::Error( "RequestLogin::ProcessRequest() - Username or password is empty" );
		return std::make_shared< ResultLogin >( this, LOGIN_REPLY::NOT_EXIST, L"" );
	}

	if( user->m_clientType == RealmClientType::CHAMPIONS_OF_NORRATH )
	{
		if( m_username == L"foo" && m_password == L"bar" )
		{
			// Retail CoN does not use any login information.
			Log::Debug( "RequestLogin : Champions of Norrath v2.0" );
		}
		else
		{
			// Network Beta CoN uses login information, but it's invalid because of version 2.0.
			Log::Debug( "RequestLogin : Champions of Norrath v1.0" );

			// TODO: Either block this, or add support for the network beta.
			return std::make_shared< ResultLogin >( this, LOGIN_REPLY::NOT_EXIST, L"" );
		}
	}
	else
	{
		// Return to Arms uses login information.
		Log::Debug( "RequestLogin : Return to Arms" );
	}

	return std::make_shared< ResultLogin >( this, LOGIN_REPLY::SUCCESS, user->m_sessionId );
}

ResultLogin::ResultLogin( GenericRequest *request, int32_t reply, std::wstring sessionId ) : GenericResponse( *request )
{
	m_reply = reply;
	m_sessionId = sessionId;
}

ByteStream &ResultLogin::Serialize()
{
	m_stream.write_u16( m_packetId );
	m_stream.write_u32( m_requestId );
	m_stream.write_u32( m_reply );

	m_stream.write_encrypted_utf16( m_sessionId );
	m_stream.write_encrypted_utf16( L"UNKNOWN DUMMY STRING" );

	return m_stream;
}
