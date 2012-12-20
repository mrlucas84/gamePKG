// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
// Misc utility modules
#include "main.h"
#include "misc.h"

/* Returns a byte of value equal to that represented by a 2 character hex
string. Ex. input of "A4" returns 164. */
unsigned char byte_from_hex_str(const char *hex_string)
{
	unsigned char outbyte = 0;
	unsigned char value1, value2 = 0;
	//puts(hex_string);
 
	if (hex_string[0] >= 48 && hex_string[0] <= 57)
		value1 = hex_string[0] - 48;
	else if (hex_string[0] >= 65 && hex_string[0] <= 70)
		value1 = hex_string[0] - 55;
 
	if (hex_string[1] >= 48 && hex_string[1] <= 57)
		value2 = hex_string[1] - 48;
	else if (hex_string[1] >= 65 && hex_string[1] <= 70)
		value2 = hex_string[1] - 55;
 
	//printf("Val1: %d, Val2: %d\n", value1, value2);
 
	outbyte = value1;
	outbyte = outbyte << 4;
	outbyte = outbyte + value2;
	//printf("Outbyte: %d\n", outbyte);
	return outbyte;
}
 
/* Writes a string of bytes to out_bytes converted from the hex values in 
hex_string. hex_string must be a length divisible by 2. Returns value 0 if 
completed successfully, 1 on error. */
int bytes_from_hex_string(unsigned char *out_bytes, const char *hex_string)
{
	if (0 != strlen(hex_string) % 2)
		return 1; //not divisible by 2!
 
	int i = 0;
	int outbyte_counter = 0;
	int len = strlen(hex_string);
	for(i=0;i<len;i+=2)
	{
		char hex_string_piece[2] = "";
		hex_string_piece[0] = *(hex_string+i);
		hex_string_piece[1] = *(hex_string+(i+1));
		*(out_bytes+outbyte_counter) = byte_from_hex_str(hex_string_piece);
		outbyte_counter++;
	}
	return 0;
}

#define addch(str, ch)			\
	str[strlen(str)]	= ch;	\
	str[strlen(str)+1]	= '\0';

char* GetNumWithCommas(int64_t nNumber) 
{
	bool bIsNegative				= false;
	char szNumber[256]				= "\0";
	char szFlippedNumber[256]		= "\0";
	char szCommaFlippedNumber[256]	= "\0";

	// Check negative number
	if(nNumber < 0) {
		bIsNegative = true;
	}

	// Avoid memory issues
	char* szCommaNumber	= (char*)malloc(256 * sizeof(char));

	// Convert integer to string
	sprintf(szNumber, "%lld", nNumber);	

	// Remove negative sign if needed
	if(bIsNegative) {
		char* pszTmp = szNumber + 1;
		sprintf(szNumber, "%s", pszTmp);
	}

	// Flip number string
	for(int nDigit = 0; nDigit < (int)strlen(szNumber); nDigit++) {
		addch(szFlippedNumber, szNumber[strlen(szNumber)-(nDigit+1)]);
	}	

	// Add commas each 3 digits
	int nDigit = 0, nCount = 0;

	while(nDigit < (int)strlen(szFlippedNumber))
	{
		if(nCount == 3) {
			addch(szCommaFlippedNumber, ',');
			nCount = 0;
			continue;
		}
		addch(szCommaFlippedNumber, szFlippedNumber[nDigit]);
		nDigit++;
		nCount++;
	}
	
	// Flip the new formatted number
	for(nDigit = 0; nDigit < (int)strlen(szCommaFlippedNumber); nDigit++) {
		addch(szCommaNumber, szCommaFlippedNumber[strlen(szCommaFlippedNumber)-(nDigit+1)]);
	}

	// Add negative sign if needed
	if(bIsNegative) {
		char szNegative[256] = "\0";
		sprintf(szNegative, "-%s", szCommaNumber);
		strcpy(szCommaNumber, szNegative);
	}

	return szCommaNumber;
}

char* toLowerCase(char* str)
{	
	int differ = 'A'-'a';
	char ch;
	int ii = strlen(str);
	for (int i=0; i <ii;i++)
	{
		strncpy(&ch, str+i, 1);
		if (ch>='A' && ch<='Z')
		{
			ch = ch-differ;
			memcpy(str+i,&ch,1);
		}
	}
	return str;
}

// lpByteSize / 1,024
#define CALCKILOBYTE(lpByteSize)	(double)((double)(lpByteSize) / (1024.0F))
// lpByteSize / 1,048,576
#define CALCMEGABYTE(lpByteSize)	(double)((double)(lpByteSize) / (1024.0F * 1024.0F))
// lpByteSize / 1,073,741,824
#define CALCGIGABYTE(lpByteSize)	(double)((double)(lpByteSize) / (1024.0F * 1024.0F * 1024.0F))
// lpByteSize / 1,099,511,627,776
#define CALCTERABYTE(lpByteSize)	(double)((double)(lpByteSize) / (1024.0F * 1024.0F * 1024.0F * 1024.0F))

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

double GetByteUnit(int64_t nSize) 
{
	// TB
	if(nSize >= (1024LL * 1024LL * 1024LL * 1024LL)) {
		return CALCTERABYTE(nSize);
	}
	// GB
	if(nSize >= (1024LL * 1024LL * 1024LL)) {
		return CALCGIGABYTE(nSize);
	}	
	// MB
	if(nSize >= (1024LL * 1024LL)) {
		return CALCMEGABYTE(nSize);
	}	
	// KB
	if(nSize >= 1024LL) {
		return CALCKILOBYTE(nSize);
	}
	return (double)nSize;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

const char* GetByteUnitStr(int64_t nSize) 
{
	// TB
	if(nSize >= (1024LL * 1024LL * 1024LL * 1024LL)) {
		return "TB";
	}		
	// GB
	if(nSize >= (1024LL * 1024LL * 1024LL)) {
		return "GB";
	}	
	// MB
	if(nSize >= (1024LL * 1024LL)) {
		return "MB";
	}	
	// KB
	if(nSize >= 1024LL) {
		return "KB";
	}
	return "Bytes";
}
