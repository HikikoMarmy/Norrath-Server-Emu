#pragma once

#include <memory>
#include <string>

#include "../GenericNetRequest.h"
#include "../GenericNetResponse.h"

class RequestGetRules : public GenericRequest
{
private:
	std::string m_language;

public:
	static std::unique_ptr< RequestGetRules > Create()
	{
		return std::make_unique< RequestGetRules >();
	}

	sptr_generic_response ProcessRequest( sptr_socket socket, sptr_byte_stream stream ) override;
	void Deserialize( sptr_byte_stream stream ) override;
};

class ResultGetRules : public GenericResponse {
private:
	std::wstring m_rules;

public:
	ResultGetRules( GenericRequest *request, std::wstring rules );
	void Serialize( ByteBuffer &out ) const;
};