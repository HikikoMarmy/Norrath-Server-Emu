#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iterator>

#include "math.h"
#include "RealmCrypt.h"

class ByteStream {
public:
	ByteStream( const std::vector< uint8_t > &data );
	ByteStream( const std::string &data );
	ByteStream( const uint8_t *data, uint32_t length );
	ByteStream( uint32_t length );
	ByteStream();

	~ByteStream();

	void resize( uint32_t size );
	void shrink_to_fit();

	template < typename T >
	void write( T value );

	template < typename T >
	T read();

	void write_u8( uint8_t value );
	void write_u16( uint16_t value );
	void write_u32( uint32_t value );
	void write_i8( int8_t value );
	void write_i16( int16_t value );
	void write_i32( int32_t value );
	void write_f32( float_t value );

	void write_utf8( const std::string &value, const size_t length = -1 );
	void write_utf16( const std::wstring &value );
	void write_sz_utf8( const std::string &value, const size_t length = -1);
	void write_sz_utf16( const std::wstring &value );
	void write_encrypted_utf8( const std::string &value );
	void write_encrypted_utf16( const std::wstring &value );

	uint8_t read_u8();
	uint16_t read_u16();
	uint32_t read_u32();
	int8_t read_i8();
	int16_t read_i16();
	int32_t read_i32();
	float_t read_f32();

	std::string read_utf8( size_t length = -1 );
	std::wstring read_utf16( size_t length = -1);
	std::string read_sz_utf8();
	std::wstring read_sz_utf16();
	std::string read_encrypted_utf8( bool hasBlockLength = true );
	std::wstring read_encrypted_utf16( bool hasBlockLength = true );

	void write_bytes( const std::vector< uint8_t > &value );
	void write_bytes( const uint8_t *value, uint32_t length );
	void write_encrypted_bytes( const std::vector< uint8_t > &value );

	std::vector< uint8_t > read_bytes( uint32_t length );
	std::vector< uint8_t > read_encrypted_bytes( uint32_t length );

	std::vector< uint8_t > get_data() const;

	uint32_t get_length() const;
	uint32_t get_position() const;
	void set_position( uint32_t pos );

	std::vector< uint8_t > data;
	uint32_t position;
};

typedef std::shared_ptr< ByteStream > sptr_byte_stream;