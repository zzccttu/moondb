/*base_64.h文件*/
#ifndef BASE64_H
#define BASE64_H

#include <string>

/**
 * Base64 编码/解码
 * @author liruixing
 */
class Base64{
private:
	static const std::string _base64_table;/*这是Base64编码使用的标准字典*/
    static const char base64_pad = '=';

public:
    /**
     * 这里必须是unsigned类型，否则编码中文的时候出错
     */
	static std::string Encode(const unsigned char * str,int bytes);
	static std::string Decode(const char *str,int bytes);
    void Debug(bool open = true);
};

#endif
