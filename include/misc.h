// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
unsigned char byte_from_hex_str(const char *hex_string);
int bytes_from_hex_string(unsigned char *out_bytes, const char *hex_string);

char* GetNumWithCommas(int64_t nNumber);
char* toLowerCase(char* str);
double GetByteUnit(int64_t nSize);
const char* GetByteUnitStr(int64_t nSize);
