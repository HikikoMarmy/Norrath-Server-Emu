#pragma once

class RequestCancelGame : public GenericRequest
{
private:
	std::wstring m_sessionId;

public:
	static std::unique_ptr< RequestCancelGame > Create()
	{
		return std::make_unique< RequestCancelGame >();
	}

	sptr_generic_response ProcessRequest( sptr_user user, sptr_byte_stream stream ) override;
	void Deserialize( sptr_byte_stream stream ) override;
};

class ResultCancelGame : public GenericResponse {
public:
	ResultCancelGame( GenericRequest *request );
	ByteStream &Serialize();
};