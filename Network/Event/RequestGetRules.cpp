#include "../../global_define.h"
#include "RequestGetRules.h"

void RequestGetRules::Deserialize( sptr_byte_stream stream )
{
	DeserializeHeader( stream );

	m_language = stream->read_sz_utf8();
}

sptr_generic_response RequestGetRules::ProcessRequest( sptr_user user, sptr_byte_stream stream )
{
	Deserialize( stream );

	// TODO: Get rules/eula based on language
	std::wstring rules =
		L"Welcome to the Norrath Emulated Server!\n\n"
		L"This server is currently in development\n"
		L"and may not be fully functional.\n\n";

	 return std::make_shared< ResultGetRules >( this, rules );
}

ResultGetRules::ResultGetRules( GenericRequest *request, std::wstring rules ) : GenericResponse( *request )
{
	m_rules = rules;
}

ByteStream &ResultGetRules::Serialize()
{
	m_stream.write_u16( m_packetId );
	m_stream.write_u32( m_requestId );
	m_stream.write_u32( 0 );

	m_stream.write_utf16( m_rules );

	return m_stream;
}
