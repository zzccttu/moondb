#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
using namespace std;
#include "crunningerror.hpp"
#include "functions.hpp"

namespace MoonDb {

class CSQLParser
{
public:
	CSQLParser();

	enum TokenType {
		T_NONE,
		T_COMMA,
		T_SEMICOLON,
		T_LEFT_BRACKET,
		T_RIGHT_BRACKET,
		T_EQUAL,
		T_NOT_EQUAL,
		T_GREATER,
		T_GREATER_EQUAL,
		T_LESS,
		T_LESS_EQUAL,
		T_AND,
		T_OR,
		T_NOT,
		T_XOR,
		T_ADDITION,
		T_MINUS,
		T_MULTIPLICATION,
		T_DIVISION,
		T_MODULO,
		T_BITWISE_AND,
		T_BITWISE_OR,
		T_BITWISE_XOR,
		T_NORMAL_STRING,
		T_HEX_STRING,
		T_BIT_NUM,
		T_CONNECT,
		T_DISCONNECT,
		T_SELECT,
		T_INSERT,
		T_UPDATE,
		T_DELETE,
		T_REPLACE,
		T_UNION,
		T_FROM,
		T_WHERE,
		T_AS,
		T_SET,
		T_INTO,
		T_VALUES,
		T_LIKE,
		T_IN,
		T_BETWEEN,
		T_JOIN,
		T_INNER,
		T_LEFT,
		T_RIGHT,
		T_CREATE,
		T_ALTER,
		T_DROP,
		T_DATABASE,
		T_TABLE,
		T_INDEX,
		T_VIEW,
		T_TRIGGER,
		T_TRANSACTION,
		T_BEGIN,
		T_COMMIT,
		T_ROLLBACK,
		T_MATCH,
		T_AGAINST,
		T_LIMIT,
		T_COUNT,
		T_MIN,
		T_MAX,
		T_AVG,
		T_SUM,
		T_SIZE
	};

	enum KeyType {
		K_NONE,
		K_KEYWORD,				/**< 关键词 */
		K_POINT,				/**< 点，对于小数点不计入，该项仅对类似指定表的字段有效 */
		K_SYMBOL,				/**< 标识符:,;() */
		K_ASSIGNMENT_OPERATOR,	/**< 赋值运算符:= */
		K_COMPARISON_OPERATOR,	/**< 比较运算符：=,!=,<,<=,<>,>,>= */
		K_LOGICAL_OPERATOR,		/**< 逻辑运算符：!,AND,OR,NOT,XOR,&&,|| */
		K_NUMERIC_OPERATOR,		/**< 数学运算符+,-,*,/,%,&,|,^ */
		K_AGGREGATE_FUNCTION,	/**< 聚合程序COUNT,MIN,MAX */
		K_NAME,					/**< 表名和字段名等 */
		K_STRING,				/**< 字符串 */
		K_INTEGER,				/**< 整数 */
		K_FLOAT,				/**< 浮点数 */
	};

	struct CToken
	{
		KeyType KType;
		TokenType TType;
		string Content;
		size_t Line;
	};

	void Parse(string& sql, vector<CToken>& tokens);

protected:
	// 经测试用map的效率比较好
	unordered_map<string, TokenType> Identifiers;
	unordered_map<string, TokenType> AggregateFunctions;
	unordered_map<char, char> SpecialCharacters;
	unordered_set<char> Seperators;

	inline bool IsWhiteSpace(const char& c)
	{
		return ' ' == c || '\t' == c || '\n' == c || '\r' == c || '\v' == c || '\f' == c;
	}

	inline bool IsWhiteSpace2(const char& c)
	{
		return ' ' == c || '\t' == c || '\r' == c || '\v' == c || '\f' == c;
	}

	inline bool IsAlphaULine(const char& c)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || '_' == c;
	}

	inline bool IsAlpaULNum(const char& c)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || '_' == c || (c >= '0' && c <= '9');
	}

	inline bool IsDigit(const char& c)
	{
		return c >= '0' && c <= '9';
	}

	bool ParseNumber(const char* &p, CToken& token);
	bool ParseHexString(const char* &p, CToken& token);
	bool ParseHexString2(const char* &p, CToken& token);
	bool ParseString(const char* &p, CToken& token);
	bool ParseBitNum(const char* &p, CToken& token);

	void PrepareErrorCode(string& code, const char* p);
	string SQLErrorCode(const string& msg, char* str1, char* str2, CToken& token);

	/*inline bool ParseNumber(const char* sqlpointer, const size_t& size, size_t& j, CToken& token)
	{
		char c2;
		bool pointexist = false;
		while(j < size) {
			c2 = *(sqlpointer + j);
			if(IsDigit(c2)) {
				token.Content.push_back(c2);
				j++;
			}
			else if('.' == c2) {
				if(pointexist) {
					return false;
				}
				else {
					pointexist = true;
					token.Content.push_back(c2);
				}
				j++;
			}
			else if('e' == c2 || 'E' == c2) {
				token.Content.push_back(c2);
				j++;
				if(j < size) {
					c2 = *(sqlpointer + j);
					if('+' == c2) {
						j++;
					}
					else if('-' == c2) {
						j++;
						token.Content.push_back(c2);
					}
					else if(!IsDigit(c2)) {
						return false;
					}
					while(j < size) {
						c2 = *(sqlpointer + j);
						if(IsDigit(c2)) {
							j++;
							token.Content.push_back(c2);
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
	}*/
};

}
