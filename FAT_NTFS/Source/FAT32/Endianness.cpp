#include "Endianness.h"

int littleEndian_1_byte_Conversion(unsigned char byte_0)
{
	int little_endian = 0;

	little_endian = byte_0 & 0xff;

	return little_endian;
}

int littleEndian_2_byte_Conversion(unsigned char byte_0, unsigned char byte_1)
{
	int little_endian = 0;

	little_endian = (byte_1 << 8) & 0xff00 |
		byte_0 & 0xff;

	return little_endian;
}

int littleEndian_3_byte_Conversion(unsigned char byte_0, unsigned char byte_1, unsigned char byte_2)
{
	int little_endian = 0;

	little_endian = (byte_2 << 16) & 0xff0000 |
		(byte_1 << 8) & 0xff00 |
		byte_0 & 0xff;

	return little_endian;
}

int littleEndian_4_byte_Conversion(unsigned char byte_0, unsigned char byte_1, unsigned char byte_2, unsigned char byte_3)
{
	int little_endian = 0;

	little_endian = (byte_3 << 24) & 0xff000000 |
		(byte_2 << 16) & 0xff0000 |
		(byte_1 << 8) & 0xff00 |
		byte_0 & 0xff;

	return little_endian;
}
