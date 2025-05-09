
#include "RealmCrypt.h"
#include "../misc/Utility.h"

RealmCrypt::RealmCrypt()
{
	std::srand( static_cast< unsigned >( std::time( nullptr ) ) );
}

std::vector< uint8_t > RealmCrypt::generateSymmetricKey( void )
{
	constexpr size_t KEY_LENGTH = 32;

	std::vector< uint8_t > keyData( KEY_LENGTH, 0 );

	// Generate 32 random bytes
	for( size_t i = 0; i < KEY_LENGTH; ++i )
	{
		keyData[ i ] = static_cast< uint8_t >( std::rand() % 255 );
	}

	return keyData;
}

std::vector<uint8_t> RealmCrypt::getSymmetricKey( void )
{
	return default_sym_key;
}

std::string RealmCrypt::encryptString( std::string &input )
{
	if( input.size() % 16 != 0 )
	{
		input.append( 16 - ( input.size() % 16 ), '\0' );
	}

	rijndael aes;

	auto result = aes.EncryptECB(
		reinterpret_cast< const uint8_t * >( input.c_str() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::string( reinterpret_cast< const char * >( result ), input.size() );
}

std::string RealmCrypt::decryptString( std::string &input )
{
	if( input.size() % 16 != 0 )
	{
		input.append( 16 - ( input.size() % 16 ), '\0' );
	}

	rijndael aes;

	auto result = aes.DecryptECB( reinterpret_cast< const uint8_t * >(
		input.c_str() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::string( reinterpret_cast< const char * >( result ), input.size() );
}

std::wstring RealmCrypt::encryptString( std::wstring &input )
{
	if( input.size() % 16 != 0 )
	{
		input.append( 16 - ( input.size() % 16 ), L'\0' );
	}

	rijndael aes;

	auto result = aes.EncryptECB( reinterpret_cast< const uint8_t * >(
		input.c_str() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::wstring( reinterpret_cast< const wchar_t * >( result ), input.size() );
}

std::wstring RealmCrypt::decryptString( std::wstring &input )
{
	if( input.size() % 16 != 0 )
	{
		input.append( 16 - ( input.size() % 16 ), L'\0' );
	}

	rijndael aes;

	auto result = aes.DecryptECB( reinterpret_cast< const uint8_t * >(
		input.c_str() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::wstring( reinterpret_cast< const wchar_t * >( result ), input.size() );
}

std::vector< uint8_t > RealmCrypt::encryptSymmetric( std::span< const uint8_t > input )
{
	if( input.size() % 16 != 0 )
	{
		std::vector< uint8_t > paddedInput( input.begin(), input.end() );
		paddedInput.resize( ( ( input.size() / 16 ) + 1 ) * 16, 0 );
		input = paddedInput;
	}

	rijndael aes;

	auto result = aes.EncryptECB( reinterpret_cast< const uint8_t * >(
		input.data() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::vector< uint8_t >( result, result + input.size() );
}

std::vector< uint8_t > RealmCrypt::decryptSymmetric( std::span< const uint8_t > input )
{
	if( input.size() % 16 != 0 )
	{
		std::vector< uint8_t > paddedInput( input.begin(), input.end() );
		paddedInput.resize( ( ( input.size() / 16 ) + 1 ) * 16, 0 );
		input = paddedInput;
	}

	rijndael aes;

	auto result = aes.DecryptECB( reinterpret_cast< const uint8_t * >(
		input.data() ),
		static_cast< uint32_t >( input.size() ),
		default_sym_key.data()
	);

	return std::vector< uint8_t >( result, result + input.size() );
}