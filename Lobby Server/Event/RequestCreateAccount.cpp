#include "../../global_define.h"
#include "RequestCreateAccount.h"

void RequestCreateAccount::Deserialize( sptr_tcp_socket socket, sptr_byte_stream stream )
{
	DeserializeHeader( stream );

	auto username = stream->read_encrypted_utf16();
	auto password = stream->read_encrypted_utf16();
	auto emailAddress = stream->read_encrypted_utf16();
	auto dateOfBirth = stream->read_encrypted_utf16();
}

sptr_generic_response RequestCreateAccount::ProcessRequest( sptr_tcp_socket socket, sptr_byte_stream stream )
{
	Deserialize( socket, stream );

	Log::Debug( "Account creation isn't supported. Random SessionID generated." );

	auto &userMng = RealmUserManager::Get();
	auto user = userMng.CreateUser( socket, L"foo", L"bar" );

	if( nullptr == user )
	{
		Log::Error( "RequestCreateAccount::ProcessRequest() - User not found!" );
		return std::make_shared< ResultCreateAccount >( this, CREATE_ACCOUNT_REPLY::ERROR_FATAL, L"" );
	}

	return std::make_shared< ResultCreateAccount >( this, CREATE_ACCOUNT_REPLY::SUCCESS, user->m_sessionId );
}

ResultCreateAccount::ResultCreateAccount( GenericRequest *request, int32_t reply, std::wstring sessionId ) : GenericResponse( *request )
{
	m_reply = reply;
	m_sessionId = sessionId;
}

ByteStream &ResultCreateAccount::Serialize()
{
	m_stream.write_u16( m_packetId );
	m_stream.write_u32( m_requestId );
	m_stream.write_u32( m_reply );

	m_stream.write_encrypted_utf16( m_sessionId );

	return m_stream;
}