#include "csqlparser.h"
#include <ctype.h>

namespace MoonDb {

CSQLParser::CSQLParser()
{
	Identifiers.emplace("AND", T_AND);
	Identifiers.emplace("OR", T_OR);
	Identifiers.emplace("NOT", T_NOT);
	Identifiers.emplace("XOR", T_XOR);
	Identifiers.emplace("CONNECT", T_CONNECT);
	Identifiers.emplace("DISCONNECT", T_DISCONNECT);
	Identifiers.emplace("SELECT", T_SELECT);
	Identifiers.emplace("INSERT", T_INSERT);
	Identifiers.emplace("UPDATE", T_UPDATE);
	Identifiers.emplace("DELETE", T_DELETE);
	Identifiers.emplace("REPLACE", T_REPLACE);
	Identifiers.emplace("UNION", T_UNION);
	Identifiers.emplace("FROM", T_FROM);
	Identifiers.emplace("WHERE", T_WHERE);
	Identifiers.emplace("AS", T_AS);
	Identifiers.emplace("SET", T_SET);
	Identifiers.emplace("INTO", T_INTO);
	Identifiers.emplace("VALUES", T_VALUES);
	Identifiers.emplace("LIKE", T_LIKE);
	Identifiers.emplace("IN", T_IN);
	Identifiers.emplace("BETWEEN", T_BETWEEN);
	Identifiers.emplace("JOIN", T_JOIN);
	Identifiers.emplace("INNER", T_INNER);
	Identifiers.emplace("LEFT", T_LEFT);
	Identifiers.emplace("RIGHT", T_RIGHT);
	Identifiers.emplace("CREATE", T_CREATE);
	Identifiers.emplace("ALTER", T_ALTER);
	Identifiers.emplace("DROP", T_DROP);
	Identifiers.emplace("DATABASE", T_DATABASE);
	Identifiers.emplace("TABLE", T_TABLE);
	Identifiers.emplace("INDEX", T_INDEX);
	Identifiers.emplace("VIEW", T_VIEW);
	Identifiers.emplace("TRIGGER", T_TRIGGER);
	Identifiers.emplace("TRANSACTION", T_TRANSACTION);
	Identifiers.emplace("BEGIN", T_BEGIN);
	Identifiers.emplace("START", T_BEGIN);
	Identifiers.emplace("COMMIT", T_COMMIT);
	Identifiers.emplace("ROLLBACK", T_ROLLBACK);
	Identifiers.emplace("MATCH", T_MATCH);
	Identifiers.emplace("AGAINST", T_AGAINST);
	Identifiers.emplace("LIMIT", T_LIMIT);
	Identifiers.emplace("TOP", T_LIMIT);

	AggregateFunctions.emplace("COUNT", T_COUNT);
	AggregateFunctions.emplace("MIN", T_MIN);
	AggregateFunctions.emplace("MAX", T_MAX);
	AggregateFunctions.emplace("AVG", T_AVG);
	AggregateFunctions.emplace("SUM", T_SUM);

	SpecialCharacters.emplace('0', '\0');
	SpecialCharacters.emplace('\'', '\'');
	SpecialCharacters.emplace('"', '"');
	SpecialCharacters.emplace('b', '\b');
	SpecialCharacters.emplace('n', '\n');
	SpecialCharacters.emplace('r', '\r');
	SpecialCharacters.emplace('t', '\t');
	SpecialCharacters.emplace('\\', '\\');

	Seperators.insert(' ');
	Seperators.insert('\t');
	Seperators.insert('\n');
	Seperators.insert('\r');
	Seperators.insert('\v');
	Seperators.insert('\f');
	Seperators.insert(',');
	Seperators.insert(';');
	Seperators.insert('(');
	Seperators.insert(')');
	Seperators.insert('=');
	Seperators.insert('!');
	Seperators.insert('>');
	Seperators.insert('<');
	Seperators.insert('+');
	Seperators.insert('-');
	Seperators.insert('*');
	Seperators.insert('/');
	Seperators.insert('%');
}

bool CSQLParser::ParseNumber(const char* &p, CToken& token)
{
	bool pointexist = false;
	while(*p) {
		if(IsDigit(*p)) {
			token.Content.push_back(*p);
			p++;
		}
		else if('.' == *p) {
			if(pointexist) {
				return false;
			}
			else {
				pointexist = true;
				token.Content.push_back(*p);
				p++;
			}
		}
		else if('e' == *p || 'E' == *p) {
			token.Content.push_back(*p);
			p++;
			if(*p) {
				if('+' == *p) {
					p++;
				}
				else if('-' == *p) {
					token.Content.push_back(*p);
					p++;
				}
				else if(!IsDigit(*p)) {
					return false;
				}
				while(*p) {
					if(IsDigit(*p)) {
						token.Content.push_back(*p);
						p++;
					}
					else {
						token.KType = K_FLOAT;
						token.TType = T_NONE;
						return true;
					}
				}
			}
			else {
				return false;
			}
		}
		else {
			break;
		}
	}
	token.TType = T_NONE;
	if(pointexist) {
		token.KType = K_FLOAT;
	}
	else {
		token.KType = K_INTEGER;
	}
	return true;
}

bool CSQLParser::ParseHexString(const char* &p, CToken& token)
{
	token.KType = K_STRING;
	token.TType = T_HEX_STRING;
	char demchar = *p;
	p++;
	bool isvalid = true;
	while(*p) {
		if(*p == demchar) {
			break;
		}
		else if(*p < '0' || (*p > '9' && *p < 'A') || (*p > 'F' && *p < 'a') || *p > 'f') {
			isvalid = false;
			break;
		}
		token.Content.push_back(*p);
		p++;
	};
	if(*p != demchar) {
		isvalid = false;
	}
	if(!isvalid || token.Content.size() % 2 == 1) {
		if(token.Content.size() > 10) {
			token.Content = token.Content.substr(0, 10) + "...";
		}
		return false;
	}
	token.Content = hex2bin(token.Content);
	p++;
	return true;
}

bool CSQLParser::ParseString(const char* &p, CToken& token)
{
	token.KType = K_STRING;
	token.TType = T_NORMAL_STRING;
	char demchar = *p;
	p++;
	bool isvalid = false;
	while(*p) {
		if(demchar == *p) {
			p++;
			isvalid = true;
			break;
		}
		else if(*p == '\\') {
			p++;
			if(*p) {
				auto it = SpecialCharacters.find(*p);
				if(it != SpecialCharacters.end()) {
					token.Content.push_back(it->second);
					p++;
				}
				else {
					break;
				}
			}
			else {
				break;
			}
		}
		else {
			token.Content.push_back(*p);
			p++;
		}
	}
	if(!isvalid) {
		if(token.Content.size() > 10) {
			token.Content = token.Content.substr(0, 10) + "...";
		}
	}
	return isvalid;
}

bool CSQLParser::ParseHexString2(const char* &p, CToken& token)
{
	token.KType = K_STRING;
	token.TType = T_HEX_STRING;
	bool isvalid = true;
	while(*p) {
		if(*p < '0' || (*p > '9' && *p < 'A') || (*p > 'F' && *p < 'a') || *p > 'f') {
			if(Seperators.find(*p) == Seperators.end()) {
				isvalid = false;
			}
			break;
		}
		token.Content.push_back(*p);
		p++;
	}
	if(!isvalid || token.Content.size() % 2 == 1) {
		if(token.Content.size() > 10) {
			token.Content = token.Content.substr(0, 10) + "...";
		}
		return false;
	}
	token.Content = hex2bin(token.Content);
	return true;
}

string CSQLParser::SQLErrorCode(const string& msg, char* str1, char* str2, CToken& token)
{
	return msg + (str1 ? string(" ") + (::strlen(str1) > 20 ? string(str1).substr(0, 20) + "..." : str1) : "") + (str2 ? string(" ") + (::strlen(str2) > 20 ? string(str2).substr(0, 20) + "..." : str2) : "") + " " + token.Content + " at line " + to_string(token.Line);
}

void CSQLParser::PrepareErrorCode(string& code, const char* p)
{
	size_t i = 0;
	while(*p && Seperators.find(*p) == Seperators.end() && i < 10) {
		code.push_back(*p);
		i++;
		p++;
	}
}

bool CSQLParser::ParseBitNum(const char* &p, CToken& token)
{
	token.KType = K_INTEGER;
	token.TType = T_BIT_NUM;
	char demchar = *p;
	p++;
	bool isvalid = true;
	while(*p) {
		if(*p == demchar) {
			break;
		}
		else if(*p != '0' && *p != '1') {
			isvalid = false;
			break;
		}
		token.Content.push_back(*p);
		p++;
	};
	if(*p != demchar) {
		isvalid = false;
	}
	if(!isvalid) {
		if(token.Content.size() > 10) {
			token.Content = token.Content.substr(0, 10) + "...";
		}
		return false;
	}
	else if(token.Content.size() > 128) {
		token.Content = token.Content.substr(0, 10) + "... is too long (maximum length is 128).";
		return false;
	}
	p++;
	return true;
}

void CSQLParser::Parse(string& sql, vector<CToken>& tokens)
{
	CToken token;
	token.Line = 1;
	bool ifsetexist = false;
	KeyType lastkeytype = K_NONE;
	string name;
	string pre;
	const char* p = &sql.front();
	char* str1 = nullptr;
	char* str2 = nullptr;
	size_t tokensize = 0;
	string typestr = "(string)...";
	while(*p) {
		// 跳过空白字符
		while(IsWhiteSpace2(*p)) {
			p++;
		}
		if('\n' == *p) {
			token.Line++;
			p++;
		}
		else {
			token.Content.clear();
			switch(*p) {
			case ',':
				token.Content.push_back(*p);
				token.KType = K_SYMBOL;
				token.TType = T_COMMA;
				p++;
				break;
			case ';':
				token.Content.push_back(*p);
				token.KType = K_SYMBOL;
				token.TType = T_SEMICOLON;
				p++;
				break;
			case '.':
				token.Content.push_back(*p);
				token.KType = K_POINT;
				token.TType = T_NONE;
				p++;
				break;
			case '(':
				token.Content.push_back(*p);
				token.KType = K_SYMBOL;
				token.TType = T_LEFT_BRACKET;
				p++;
				break;
			case ')':
				token.Content.push_back(*p);
				token.KType = K_SYMBOL;
				token.TType = T_RIGHT_BRACKET;
				p++;
				break;
			case '=':
				token.Content.push_back(*p);
				if(ifsetexist) {
					token.KType = K_ASSIGNMENT_OPERATOR;
				}
				else {
					token.KType = K_COMPARISON_OPERATOR;
				}
				token.TType = T_EQUAL;
				p++;
				break;
			case '!':
				token.Content.push_back(*p);
				p++;
				if('=' == *p) {
					token.Content.push_back(*p);
					token.KType = K_COMPARISON_OPERATOR;
					token.TType = T_NOT_EQUAL;
					p++;
				}
				else {
					token.KType = K_LOGICAL_OPERATOR;
					token.TType = T_NOT;
				}
				break;
			case '>':
				token.Content.push_back(*p);
				p++;
				token.KType = K_COMPARISON_OPERATOR;
				if('=' == *p) {
					token.Content.push_back(*p);
					token.TType = T_GREATER_EQUAL;
					p++;
				}
				else {
					token.TType = T_GREATER;
				}
				break;
			case '<':
				token.Content.push_back(*p);
				p++;
				token.KType = K_COMPARISON_OPERATOR;
				if('=' == *p) {
					token.Content.push_back(*p);
					token.TType = T_LESS_EQUAL;
					p++;
				}
				if('>' == *p) {
					token.Content.push_back(*p);
					token.TType = T_NOT_EQUAL;
					p++;
				}
				else {
					token.TType = T_LESS;
				}
				break;
			case '+':
				token.Content.push_back(*p);
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_ADDITION;
				p++;
				break;
			case '-':
				token.Content.push_back(*p);
				p++;
				// 匹配负数
				if(IsDigit(*p) && (K_KEYWORD == lastkeytype || K_SYMBOL == lastkeytype || K_ASSIGNMENT_OPERATOR == lastkeytype ||
					K_COMPARISON_OPERATOR == lastkeytype || K_LOGICAL_OPERATOR == lastkeytype || K_NUMERIC_OPERATOR == lastkeytype)) {
					if(!ParseNumber(p, token)) {
						PrepareErrorCode(token.Content, p);
						ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong number near", str1, str2, token));
					}
				}
				else {
					token.KType = K_NUMERIC_OPERATOR;
					token.TType = T_MINUS;
					p++;
				}
				break;
			case '*':
				token.Content.push_back(*p);
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_MULTIPLICATION;
				p++;
				break;
			case '/':
				token.Content.push_back(*p);
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_DIVISION;
				p++;
				break;
			case '%':
				token.Content.push_back(*p);
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_MODULO;
				p++;
				break;
			case '&':
				if('&' == *(p + 1)) {
					token.Content = "&&";
					token.KType = K_LOGICAL_OPERATOR;
					token.TType = T_AND;
					p += 2;
				}
				else {
					token.Content.push_back(*p);
					token.KType = K_NUMERIC_OPERATOR;
					token.TType = T_BITWISE_AND;
					p++;
				}
				break;
			case '|':
				if('|' == *(p + 1)) {
					token.Content = "||";
					token.KType = K_LOGICAL_OPERATOR;
					token.TType = T_OR;
					p += 2;
				}
				else {
					token.Content.push_back(*p);
					token.KType = K_NUMERIC_OPERATOR;
					token.TType = T_BITWISE_OR;
					p++;
				}
				break;
			case '^':
				token.Content.push_back(*p);
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_BITWISE_XOR;
				p++;
				break;
			case '"':
			case '\'':
				// 匹配字符串（单引号或双引号）
				pre.resize(1);
				pre[0] = *p;
				if(!ParseString(p, token)) {
					PrepareErrorCode(token.Content, p);
					ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: an incomplete or unquoted string near", str1, str2, token));
				}
				break;
			default:
				// 匹配诸如x'b9e6b6a8'的十六进制字符串
				if('x' == ::tolower(*p) && ('\'' == *(p + 1) || '"' == *(p + 1))) {
					pre.resize(2);
					pre[0] = *p;
					pre[1] = *(p + 1);
					p++;
					if(!ParseHexString(p, token)) {
						token.Content = pre + token.Content;
						PrepareErrorCode(token.Content, p);
						ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong hex string near", str1, str2, token));
					}
				}
				// 十六进制字符串，匹配类似0xb9e6b6a8的情况
				else if('0' == *p && ::tolower(*(p + 1)) == 'x') {
					pre.resize(2);
					pre[0] = *p;
					pre[1] = *(p + 1);
					p += 2;
					if(!ParseHexString2(p, token)) {
						token.Content = pre + token.Content;
						PrepareErrorCode(token.Content, p);
						ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong hex string near", str1, str2, token));
					}
				}
				// 对应二进制BIT字段，如：b'000101'
				else if('b' == ::tolower(*p) && ('\'' == *(p + 1) || '"' == *(p + 1))) {
					pre.resize(2);
					pre[0] = *p;
					pre[1] = *(p + 1);
					p++;
					if(!ParseBitNum(p, token)) {
						token.Content = pre + token.Content;
						PrepareErrorCode(token.Content, p);
						ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong bit num near", str1, str2, token));
					}
				}
				// 判断是否关键字或表名及字段名等标识符
				else if(IsAlphaULine(*p)) {
					do {
						token.Content.push_back(*p);
						p++;
					} while(IsAlpaULNum(*p));
					name = to_upper_copy(token.Content);
					// 看是否为关键词
					auto it = Identifiers.find(name);
					if(it != Identifiers.end()) {
						token.KType = K_KEYWORD;
						token.TType = it->second;
						if(T_SET == token.TType) {
							ifsetexist = true;
						}
						else if(T_WHERE == token.TType) {
							ifsetexist = false;
						}
					}
					else {
						auto it = AggregateFunctions.find(name);
						if(it != AggregateFunctions.end()) {
							token.KType = K_AGGREGATE_FUNCTION;
							token.TType = it->second;
						}
						else {
							token.KType = K_NAME;
							token.TType = T_NONE;
						}
					}
				}
				else if(*p >= '0' && *p <= '9') {
					if(!ParseNumber(p, token)) {
						PrepareErrorCode(token.Content, p);
						ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong number near", str1, str2, token));
					}
				}
				// 其他情况错误
				else {
					PrepareErrorCode(token.Content, p);
					ThrowError(ERR_INVALID_SQL, SQLErrorCode("An error in your SQL syntax: wrong char near", str1, str2, token));
				}
			}
			tokens.push_back(token);
			lastkeytype = token.KType;
			tokensize++;
			if(T_HEX_STRING == token.TType) {
				str2 = &typestr.front();
			}
			else {
				str2 = &tokens[tokensize - 1].Content.front();
			}
			if(tokensize > 1) {
				if(T_HEX_STRING == tokens[tokensize - 2].TType) {
					str1 = &typestr.front();
				}
				else {
					str1 = &tokens[tokensize - 2].Content.front();
				}
			}
		}
	}

	/*size_t size = sql.size();
	const char* sqlpointer = &sql.front();
	CToken token;
	string name;
	size_t i = 0;
	size_t j = 0;
	size_t lastlinepos = 0;
	char c, c2;
	token.Line = 1;
	bool ifsetexist = false;
	KeyType lastkeytype = K_NONE;
	while(i < size) {
		c = *(sqlpointer + i);
		if(IsWhiteSpace(c)) {
			if('\n' == c) {
				token.Line++;
				j = i + 1;
				while(j < size && IsWhiteSpace2(*(sqlpointer + j))) {
					j++;
				}
				lastlinepos = j;
				i = j;
				continue;
			}
			i++;
		}
		else {
			token.Position = i + 1 - lastlinepos;
			token.Content.clear();
			if(',' == c) {
				token.KType = K_SYMBOL;
				token.TType = T_COMMA;
				i++;
			}
			else if(';' == c) {
				token.KType = K_SYMBOL;
				token.TType = T_SEMICOLON;
				i++;
			}
			else if('.' == c) {
				token.KType = K_POINT;
				token.TType = T_NONE;
				i++;
			}
			else if('(' == c) {
				token.KType = K_SYMBOL;
				token.TType = T_LEFT_BRACKET;
				i++;
			}
			else if(')' == c) {
				token.KType = K_SYMBOL;
				token.TType = T_RIGHT_BRACKET;
				i++;
			}
			else if('=' == c) {
				if(ifsetexist) {
					token.KType = K_ASSIGNMENT_OPERATOR;
				}
				else {
					token.KType = K_COMPARISON_OPERATOR;
				}
				token.TType = T_EQUAL;
				i++;
			}
			else if('!' == c) {
				if(i + 1 < size && '=' == *(sqlpointer + i + 1)) {
					token.KType = K_COMPARISON_OPERATOR;
					token.TType = T_NOT_EQUAL;
					i += 2;
				}
				else {
					token.KType = K_LOGICAL_OPERATOR;
					token.TType = T_NOT;
					i++;
				}
			}
			else if('>' == c) {
				token.KType = K_COMPARISON_OPERATOR;
				if(i + 1 < size && '=' == *(sqlpointer + i + 1)) {
					token.TType = T_GREATER_EQUAL;
					i += 2;
				}
				else {
					token.TType = T_GREATER;
					i++;
				}
			}
			else if('<' == c) {
				token.KType = K_COMPARISON_OPERATOR;
				if(i + 1 < size && '=' == *(sqlpointer + i + 1)) {
					token.TType = T_LESS_EQUAL;
					i += 2;
				}
				if(i + 1 < size && '>' == *(sqlpointer + i + 1)) {
					token.TType = T_NOT_EQUAL;
					i += 2;
				}
				else {
					token.TType = T_LESS;
					i++;
				}
			}
			else if('+' == c) {
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_ADDITION;
				i++;
			}
			else if('-' == c) {
				// 匹配负数
				if(i + 1 < size && IsDigit(*(sqlpointer + i + 1)) && (K_KEYWORD == lastkeytype || K_SYMBOL == lastkeytype || K_ASSIGNMENT_OPERATOR == lastkeytype || K_COMPARISON_OPERATOR == lastkeytype ||
					K_LOGICAL_OPERATOR == lastkeytype || K_NUMERIC_OPERATOR == lastkeytype)) {
					token.Content.push_back(c);
					j = i + 1;
					bool valid = ParseNumber(sqlpointer, size, j, token);
					i = j;
					if(!valid) {
						ThrowError(ERR_INVALID_SQL, "An error in your SQL syntax: wrong number near " + sql.substr(i, j - i + 1) + " at position " + to_string(token.Position) + " at line " + to_string(token.Line));
					}
				}
				else {
					token.KType = K_NUMERIC_OPERATOR;
					token.TType = T_MINUS;
					i++;
				}
			}
			else if('*' == c) {
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_MULTIPLICATION;
				i++;
			}
			else if('/' == c) {
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_DIVISION;
				i++;
			}
			else if('%' == c) {
				token.KType = K_NUMERIC_OPERATOR;
				token.TType = T_MODULO;
				i++;
			}
			// 匹配诸如x'b9e6b6a8'的十六进制字符串
			else if('x' == ::tolower(c) && i + 2 < size && ('\'' == *(sqlpointer + i + 1) || '"' == *(sqlpointer + i + 1))) {
				token.KType = K_STRING;
				token.TType = T_NONE;
				j = i + 2;
				bool isvalid = true;
				char demchar = *(sqlpointer + i + 1);
				do {
					c2 = *(sqlpointer + j);
					if(c2 == demchar) {
						break;
					}
					else if(c2 < '0' || (c2 > '9' && c2 < 'A') || (c2 > 'F' && c2 < 'a') || c2 > 'f') {
						isvalid = false;
						break;
					}
					j++;
				} while(j < size);
				if(c2 != demchar) {
					isvalid = false;
				}
				token.Content = sql.substr(i + 2, j - i - 2);
				if(!isvalid || token.Content.size() % 2 == 1) {
					ThrowError(ERR_INVALID_SQL, "An error in your SQL syntax: wrong hex string near " + sql.substr(i, j - i + 1) + " at position " + to_string(token.Position) + " at line " + to_string(token.Line));
				}
				token.Content = hex2bin(token.Content);
				i = j + 1;
			}
			// 判断是否关键字或表名及字段名等标识符
			else if(IsAlphaULine(c)) {
				j = i + 1;
				while(j < size) {
					if(IsAlpaULNum(*(sqlpointer + j))) {
						j++;
					}
					else {
						break;
					}
				}
				token.Content = sql.substr(i, j - i);
				name = to_upper_copy(token.Content);
				// 看是否为关键词
				auto it = Identifiers.find(name);
				if(it != Identifiers.end()) {
					token.KType = K_KEYWORD;
					token.TType = it->second;
					if(T_SET == token.TType) {
						ifsetexist = true;
					}
					else if(T_WHERE == token.TType) {
						ifsetexist = false;
					}
				}
				else {
					auto it = AggregateFunctions.find(name);
					if(it != AggregateFunctions.end()) {
						token.KType = K_AGGREGATE_FUNCTION;
						token.TType = it->second;
					}
					else {
						auto it = LogicalOperators.find(name);
						if(it != LogicalOperators.end()) {
							token.KType = K_LOGICAL_OPERATOR;
							token.TType = it->second;
						}
						else {
							token.KType = K_NAME;
							token.TType = T_NONE;
						}
					}
				}
				i = j;
			}
			// 匹配字符串（单引号或双引号）
			else if('\'' == c || '"' == c) {
				j = i + 1;
				bool isvalid = false;
				while(j < size) {
					c2 = *(sqlpointer + j);
					if(c2 == c) {
						j++;
						isvalid = true;
						break;
					}
					else if(c2 == '\\') {
						if(j + 1 < size) {
							auto it = SpecialCharacters.find(*(sqlpointer + j + 1));
							if(it != SpecialCharacters.end()) {
								token.Content.push_back(it->second);
								j += 2;
							}
							else {
								break;
							}
						}
						else {
							break;
						}
					}
					else {
						token.Content.push_back(c2);
						j++;
					}
				}
				if(isvalid) {
					token.KType = K_STRING;
					token.TType = T_NONE;
				}
				else {
					ThrowError(ERR_INVALID_SQL, "An error in your SQL syntax: an incomplete string near " + sql.substr(i, j - i + 1) + " at position " + to_string(token.Position) + " at line " + to_string(token.Line));
				}
				i = j;
			}
			// 十六进制字符串，匹配类似0xb9e6b6a8的情况
			else if('0' == c && i + 1 < size && ::tolower(*(sqlpointer + i + 1)) == 'x') {
				token.KType = K_STRING;
				token.TType = T_NONE;
				j = i + 2;
				bool isvalid = true;
				while(j < size) {
					c2 = *(sqlpointer + j);
					if(c2 < '0' || (c2 > '9' && c2 < 'A') || (c2 > 'F' && c2 < 'a') || c2 > 'f') {
						if(Seperators.find(c2) == Seperators.end()) {
							isvalid = false;
						}
						break;
					}
					j++;
				}
				token.Content = sql.substr(i + 2, j - i - 2);
				if(!isvalid || token.Content.size() % 2 == 1) {
					ThrowError(ERR_INVALID_SQL, "An error in your SQL syntax: wrong hex string near " + sql.substr(i, j - i) + " at position " + to_string(token.Position) + " at line " + to_string(token.Line));
				}
				token.Content = hex2bin(token.Content);
				i = j;
			}
			else if(c >= '0' && c <= '9') {
				token.Content.push_back(c);
				j = i + 1;
				bool valid = ParseNumber(sqlpointer, size, j, token);
				i = j;
				if(!valid) {
					ThrowError(ERR_INVALID_SQL, "An error in your SQL syntax: wrong number near " + sql.substr(i, j - i + 1) + " at position " + to_string(token.Position) + " at line " + to_string(token.Line));
				}
			}
			// 其他情况错误
			else {
				break;
			}
			tokens.push_back(token);
			lastkeytype = token.KType;
		}
	}*/
}

}
