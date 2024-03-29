#include "parser.hpp"

extern int mdi;
extern Data mydata[];
std::string MAX_UINT64 = std::to_string((uint64_t) -1);
// 18446744073709551615

void TokenParser::Parse(const std::string &line)
{
	for( unsigned int i = 0; i < line.length(); ) 
	{
		i = mydata[mdi].Store( line, i );
	}
}

int TokenParser::SetStartCallback( void (*f)() )
{
	f();
	return( 0 );
}

int TokenParser::SetEndCallback( void (*f)() )
{
	f();
	return( 0 );
}

void Data::ShowValue( char s1[], char s2[] )
{
	if( len == 0 )
	{
		std::cout << s2[0] << num << s2[1];
	}
	else
	{
		std::cout << s1[0] << txt << s1[1];
	}
	std::cout << " ";
}

void ShowCounters()
{
	std::cout << mdi << " strings/numbers buffered. Limit: " << MAX_DATA << std::endl;
}

void ShowValues()
{
	for( int i = 0; i < mdi; i++ )
		mydata[i].ShowValue( (char *) "()", (char *) "##" );
	if ( mdi > 0 )
		std::cout << std::endl;
}

unsigned int Data::Store( std::string str, unsigned int pos)
{																																								
	std::locale	loc;
	bool isnumber = true;
	unsigned int fin, numpos;

	while( std::isspace( str[pos], loc ) && str.length() > pos )
	{
		pos++;
	}
	fin = pos;
	while( !std::isspace(str[fin], loc ) && fin < str.length() )
	{
		if( ! std::isdigit( str[fin], loc ) )
		{
			isnumber = false;
		}
		fin++;
	}

	// Check whether number fits limit
	if( fin != pos )
	{
		if( isnumber )
		{
			// Save pos to keep leading zeros if digits should be parsed as string
			numpos = pos;
			// Truncate leading zeros
			while(str[numpos] == '0' && str.length() > 1)
				numpos++;
			if( fin - numpos > MAX_UINT64.length() || 
				( fin - numpos == MAX_UINT64.length() && (MAX_UINT64.compare( str.substr(pos, fin - numpos) ) < 0) ) )
			{
				isnumber = false;
			}
			
		}
		if( isnumber )
		{
			std::istringstream iss( str.substr(pos, fin - numpos) );

			iss >> num;
			len = 0;
			mdi++;
		}
		else
		{
			if( fin > str.length() )
			{
				fin = str.length();
			}
			txt = str.substr( pos, fin - pos);
			len = fin - pos;
			if( len > 0 )
			{
				mdi++;
			}
		}
		// Rewind to next nonblanc symbol
		while( std::isspace(str[fin]) && fin < str.length() )
		{
			fin++;
		}
	}
	return( fin );
}

typedef int (*MyFuncPtr)(uint64_t);
int TokenParser::SetDigitTokenCallback( MyFuncPtr f )
{
	int sum = 0;
	for( int i = 0; i < mdi; i++ )
		if( mydata[i].len == 0 )
			sum += f( mydata[i].num );
	if( sum != 0 )
		std::cout << "\nTotal count: " << sum << std::endl;
	return( 0 );
}

int ShowOdd( uint64_t val )
{
	if( val % 2 > 0 )
	{
		std::cout << val << " ";
		return( 1 );
	}
	else
		return( 0 );
}

int ShowEven( uint64_t val )
{
	if( val % 2 == 0 )
	{
		std::cout << val << " ";
		return( 1 );
	}
	else
		return( 0 );
}
