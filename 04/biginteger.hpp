#pragma once

#include <iostream>
#include <cassert>
#include <stdexcept>
#define	PLUS	'+'
#define ZERO	'0'
#define	MINUS	'-'
#define NONE	'\0'

#define	BASE		10	// Основание системы счисления
#define	BASE_TYPE	int

class BigInt
{
public:
	BigInt();
	BigInt(std::string);
	BigInt(int32_t);
	BigInt(const BigInt &rhs);
	BigInt(BigInt&& rhs);

	void zero_data();
	void no_leading_zeros();
	~BigInt();
	void Debug();
//	friend std::ostream& operator <<(std::ostream&, const BigInt&);
	// Арифметические операторы 
	BigInt operator +(const BigInt&);
	BigInt operator +(int32_t);
	BigInt operator -(const BigInt&) const;
	BigInt operator -(int32_t);
	BigInt operator *(const BigInt&);
	BigInt operator *(int32_t);
	// Унарный минус
	BigInt operator -();
	// Логические операторы
	bool operator ==(const BigInt&) const;
	bool operator !=(const BigInt&) const;
	bool operator <(const BigInt&) const;        
	bool operator >=(const BigInt&) const;
	bool operator <=(const BigInt&) const;
	bool operator >(const BigInt&) const;
	BigInt operator =(const BigInt &rhs);
	BigInt& operator =(BigInt &&);


	friend std::ostream &operator<<(std::ostream &os, const BigInt &m) {
		if ( m.sign == MINUS )
			os << '-';
		for (size_t i=0; i < m.len; ++i) {
			os << m.data[i];
		}
		return os;
	}
/*
	friend std::copy(const BigInt &m) {
		if(data != nullptr)
			delete data;
	}
*/
	
 
private:
	BASE_TYPE	*data  = nullptr;	// данные
	size_t		len = 0;	// размер данных
	char		sign = 0;	// знак
};
