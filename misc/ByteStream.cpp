#include <codecvt>
#include "ByteStream.h"
#include <span>

ByteStream::ByteStream( const std::vector< uint8_t > &data )
{
	this->data = data;
	this->position = 0;
}

ByteStream::ByteStream( const std::string &data )
{
	this->data = std::vector< uint8_t >( data.begin(), data.end() );
	this->position = 0;
}

ByteStream::ByteStream( const uint8_t *data, uint32_t length )
{
	this->data = std::vector< uint8_t >( data, data + length );
	this->position = 0;
}

ByteStream::ByteStream( uint32_t length )
{
	this->data = std::vector< uint8_t >( length, 0 );
	this->position = 0;
}

ByteStream::ByteStream()
{
	this->position = 0;
}

ByteStream::~ByteStream()
{
}

void ByteStream::resize( uint32_t size )
{
	data.resize( size );
}

void ByteStream::shrink_to_fit()
{
	data.shrink_to_fit();
}

template < typename T >
void ByteStream::write( T value )
{
	write_bytes( ( uint8_t * )&value, sizeof( T ) );
}

template < typename T >
T ByteStream::read()
{
	T value = *( T * )&data[ position ];
	position += sizeof( T );

	return value;
}

void ByteStream::write_utf8( const std::string &str, std::optional<uint32_t> length )
{
	if( length.has_value() )
	{
		write_u32( length.value() );

		if( length.value() > str.size() )
		{
			write_bytes( std::vector< uint8_t >( str.begin(), str.end() ) );
			write_bytes( std::vector< uint8_t >( length.value() - str.size(), 0 ) );
		}
		else
		{
			write_bytes( std::vector< uint8_t >( str.begin(), str.begin() + length.value() ) );
		}
	}
	else
	{
		write_u32( static_cast< uint32_t >( str.size() ) );
		write_bytes( std::vector< uint8_t >( str.begin(), str.end() ) );
	}
}

void ByteStream::write_utf16( const std::wstring &str, std::optional<uint32_t> length )
{
	write_u32( static_cast< uint32_t >( str.size() ) + 1 );

	for( wchar_t ch : str )
	{
		uint16_t val = static_cast< uint16_t >( ch );
		write<uint8_t>( val & 0xFF );
		write<uint8_t>( ( val >> 8 ) & 0xFF );
	}

	write_u16( 0 );
}

void ByteStream::write_sz_utf8( const std::string &str, std::optional<uint32_t> length )
{
	if( length )
	{
		write_bytes( std::vector< uint8_t >( str.begin(), str.end() ) );
		write_bytes( std::vector< uint8_t >( length.value() - str.size(), 0 ) );
	}
	else
	{
		write_bytes( std::vector< uint8_t >( str.begin(), str.end() ) );
		write< uint8_t >( 0 );
	}
}

void ByteStream::write_sz_utf16( const std::wstring &str, std::optional<uint32_t> length )
{
	for( wchar_t ch : str )
	{
		uint16_t val = static_cast< uint16_t >( ch );
		write<uint8_t>( val & 0xFF );
		write<uint8_t>( ( val >> 8 ) & 0xFF );
	}

	if( length )
	{
		size_t bytesWritten = str.size() * 2;
		size_t totalBytes = length.value();

		if( bytesWritten < totalBytes )
		{
			write_bytes( std::vector<uint8_t>( totalBytes - bytesWritten, 0 ) );
		}
	}
	else
	{
		write<uint16_t>( 0 );
	}
}

void ByteStream::write_encrypted_utf8( const std::string &str )
{
	auto encrypted = RealmCrypt::encryptSymmetric( std::vector< uint8_t >( str.begin(), str.end() ) );

	write_u32( static_cast< uint32_t >( encrypted.size() ) + 4 );
	write_u32( static_cast< uint32_t >( str.size() ) );

	write_bytes( encrypted );
}

void ByteStream::write_encrypted_utf16( const std::wstring &str )
{
	std::vector< uint8_t > utf16;
	for( auto c : str )
	{
		utf16.push_back( c & 0xFF );
		utf16.push_back( ( c >> 8 ) & 0xFF );
	}

	auto encrypted = RealmCrypt::encryptSymmetric( utf16 );
	uint32_t encryptedLength = static_cast< uint32_t >( encrypted.size() );
	uint32_t decryptedLength = static_cast< uint32_t >( utf16.size() );

	write_u32( ( encryptedLength + 4 ) / 2 );
	write_u32( decryptedLength );

	write_bytes( encrypted );
}

uint8_t ByteStream::read_u8()
{
	return read< uint8_t >();
}

uint16_t ByteStream::read_u16()
{
	return read< uint16_t >();
}

uint32_t ByteStream::read_u32()
{
	return read< uint32_t >();
}

int8_t ByteStream::read_i8()
{
	return read< int8_t >();
}

int16_t ByteStream::read_i16()
{
	return read< int16_t >();
}

int32_t ByteStream::read_i32()
{
	return read< int32_t >();
}

float_t ByteStream::read_f32()
{
	return read< float_t >();
}

std::string ByteStream::read_utf8( std::optional<uint32_t> length )
{
	if( !length )
	{
		length = read_u32();
	}

	if( position + length.value() > data.size() )
	{
		throw std::runtime_error( "read_utf8: Attempt to read past end of buffer" );
	}

	std::string value( reinterpret_cast< const char * >( &data[ position ] ), length.value() );
	position += length.value();

	return value;
}

std::wstring ByteStream::read_utf16( std::optional<uint32_t> length )
{
	if( !length )
	{
		length = read_u32();
	}

	uint32_t byteLength = length.value() * 2;

	if( position + byteLength > data.size() )
	{
		throw std::runtime_error( "read_utf16: Attempt to read past end of buffer" );
	}

	std::wstring value;
	value.reserve( length.value() );

	for( size_t i = 0; i < byteLength; i += 2 )
	{
		uint16_t ch = data[ position + i ] | ( data[ position + i + 1 ] << 8 );
		value.push_back( static_cast< wchar_t >( ch ) );
	}

	position += byteLength;
	return value;
}

std::string ByteStream::read_sz_utf8()
{
	std::string value;
	while( data[ position ] != 0 )
	{
		value.push_back( data[ position ] );
		position++;
	}

	position++;

	return value;
}

std::wstring ByteStream::read_sz_utf16()
{
	std::wstring value;
	while( data[ position ] != 0 || data[ position + 1 ] != 0 )
	{
		value.push_back( data[ position ] | ( data[ position + 1 ] << 8 ) );
		position += 2;
	}

	position += 2;

	return value;
}

std::string ByteStream::read_encrypted_utf8( bool hasBlockLength )
{
	uint32_t encryptedLength = 0;
	uint32_t decryptedLength = 0;

	if( hasBlockLength )
	{
		uint32_t blockLength = read_u32() * 2;
		decryptedLength = read_u32();
		encryptedLength = blockLength - 4;
	}
	else
	{
		decryptedLength = read_u32();
		encryptedLength = Utility::round_up( decryptedLength, 16 );
	}

	std::span< const uint8_t > encryptedBuffer( data.data() + position, encryptedLength );

	position += encryptedLength;

	if( decryptedLength == 0 )
	{
		return "";
	}

	// Decrypt the buffer
	std::vector< uint8_t > decryptedBuffer = RealmCrypt::decryptSymmetric( encryptedBuffer );

	std::string result( decryptedBuffer.begin(), decryptedBuffer.end() );

	return result;
}

std::wstring ByteStream::read_encrypted_utf16( bool hasBlockLength )
{
	uint32_t encryptedLength = 0;
	uint32_t decryptedLength = 0;

	if( hasBlockLength )
	{
		uint32_t blockLength = read_u32() * 2;
		decryptedLength = read_u32();	// This length is already multiplied by sizeof(wchar_t)
		encryptedLength = blockLength - 4;
	}
	else
	{
		decryptedLength = read_u32();
		encryptedLength = Utility::round_up( decryptedLength, 16 );
	}

	std::span< const uint8_t > encryptedBuffer( data.data() + position, encryptedLength );

	position += encryptedLength;

	if( decryptedLength == 0 )
	{
		return L"";
	}

	// Decrypt the buffer
	std::vector< uint8_t > decryptedBuffer = RealmCrypt::decryptSymmetric( encryptedBuffer );

	std::wstring result( decryptedLength / 2, L'\0' );
	std::memcpy( result.data(), decryptedBuffer.data(), decryptedLength );

	return result;
}

void ByteStream::write_bytes( const std::vector< uint8_t > &value )
{
	std::copy( value.begin(), value.end(), std::back_inserter( data ) );
	position += value.size();
}

void ByteStream::write_bytes( const uint8_t *value, uint32_t length )
{
	std::copy( value, value + length, std::back_inserter( data ) );
	position += length;
}

void ByteStream::write_encrypted_bytes( const std::vector<uint8_t> &value )
{
	auto encrypted = RealmCrypt::encryptSymmetric( value );

	write_u32( static_cast< uint32_t >( encrypted.size() ) + 4 );
	write_u32( static_cast< uint32_t >( value.size() ) );

	write_bytes( encrypted );
}

std::vector<uint8_t> ByteStream::read_bytes( uint32_t length )
{
	std::vector<uint8_t> value( length, 0 );

	std::copy( data.begin() + position, data.begin() + position + length, value.begin() );

	position += length;

	return value;
}

std::vector<uint8_t> ByteStream::read_encrypted_bytes( uint32_t length )
{
	std::vector< uint8_t > encrypted = read_bytes( length );

	auto decrypted = RealmCrypt::decryptSymmetric( encrypted );

	return decrypted;
}

std::vector<uint8_t> ByteStream::get_data() const
{
	return data;
}

size_t ByteStream::get_length() const
{
	return data.size();
}

void ByteStream::set_position( size_t where )
{
	if( where > data.size() )
	{
		where = data.size();
	}

	this->position = where;
}

size_t ByteStream::get_position() const
{
	return this->position;
}

void ByteStream::write_u8( uint8_t value )
{
	write< uint8_t >( value );
}

void ByteStream::write_u16( uint16_t value )
{
	write< uint16_t >( value );
}

void ByteStream::write_u32( uint32_t value )
{
	write< uint32_t >( value );
}

void ByteStream::write_i8( int8_t value )
{
	write< int8_t >( value );
}

void ByteStream::write_i16( int16_t value )
{
	write< int16_t >( value );
}

void ByteStream::write_i32( int32_t value )
{
	write< int32_t >( value );
}

void ByteStream::write_f32( float_t value )
{
	write< float_t >( value );
}
