#include "util.hh"

/*
If QQIC < 128, QQI = QQIC
If QQIC >= 128, QQIC represents a floating-point value as follows:
0  1 2 3  4 5 6 7
+-+-+-+-+-+-+-+-+
|1| exp |  mant  |
+-+-+-+-+-+-+-+-+
QQI = (mant | 0x10) << (exp + 3)
*/

MiniFloat::MiniFloat(unsigned int i) {
	set(i);
}

MiniFloat::MiniFloat(const MiniFloat& other): byte(other.byte) {}

void MiniFloat::set(unsigned int qqi) {
	if (qqi < 128) {
		this->byte = (uint8_t) qqi;
	} else if (qqi > 31744) {
		throw std::invalid_argument("MiniFloat value overflow");
	} else {
		unsigned int exp = 0;
		qqi >>= 3; //remove the +3 part from the exp from QQIC
		while (qqi > 31) {
			++exp;
			qqi >>= 1;
		}
		exp <<= 4; //adjust the exp so that it's bits are in pos 1 -> 3 (MSB = 0)
		uint8_t code = 0x80 | exp | ((uint8_t) qqi & 0x0F);
		this->byte = code;
	}
}

unsigned int MiniFloat::get() {
	uint8_t bit = byte & (1<<7);

	if (!(this->byte & (0x1 << 7))) {
		return this->get_int();
	} else {
		uint8_t mant = this->byte & 0x0F;
		uint8_t exp = (this->byte & 0x70) >> 4;
		unsigned int QQI = (mant | 0x10) << (exp + 3);
		return QQI;
	}
}

unsigned int MiniFloat::get_int(){
	if (this->byte > 128) {
		throw std::invalid_argument("Called get_int while MiniFloat is a float value");
	} else {
		return this->byte;
	}
}
