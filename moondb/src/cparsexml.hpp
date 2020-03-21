#pragma once

#include <string>
#include <map>
#include <fstream>
#include "csystemerror.hpp"

namespace MoonDb
{
	class CParseXML
	{
	public:
		class CNode
		{
		public:
			inline CNode& operator[] (const std::string& key)
			{
				return this->children[key];
			}

			bool isValue;
			bool read;
			std::string content;
			std::map<std::string, CNode> children;
		};

		static void Read(const char* filename, CNode& xml_node)
		{
			std::ifstream ifs;
			ifs.open(filename, std::ifstream::in);
			if (ifs.fail()) {
				char buf[1024];
				sprintf(buf, "Failed to open the file(%s). ", filename);
				TriggerError(buf);
			}

			ifs.seekg(0, ifs.end);
			size_t file_length = static_cast<size_t>(ifs.tellg());
			ifs.seekg(0, ifs.beg);

			char * buffer = new char[file_length + 1];
			ifs.read(buffer, static_cast<std::streamsize>(file_length));
			ifs.close();
			buffer[file_length] = '\0';
			for (size_t i = 0; i < file_length; i++) {
				if (isSpace(buffer[i])) {
					continue;
				}
				parse(buffer, i, file_length, xml_node);
				break;
			}

			delete[] buffer;
		}

	protected:
		static bool isSpace(char c) noexcept
		{
			return 9 == c || 10 == c || 11 == c || 13 == c || 32 == c;
		}

		static size_t parse(const char* buffer, size_t position, size_t file_length, CNode& xml_node)
		{
			if (position >= file_length) {
				return file_length;
			}

			xml_node.read = false;

			// 如果是内容
			if ('<' != buffer[position]) {
				std::string content;
				size_t curpos = position;
				for (size_t i = position; i < file_length; i++) {
					if ('<' == buffer[i]) {
						curpos = i;
						break;
					}
					content += buffer[i];
					curpos++;
				}
				xml_node.content = content;
				xml_node.isValue = true;
				return curpos;
			}

			// 如果是节点
			xml_node.isValue = false;

			std::string key;
			// 取得key
			for (size_t i = position + 1; i < file_length; i++) {
				if ('>' == buffer[i]) {
					position = i + 1;
					break;
				}
				key += buffer[i];
			}

			while (position + 1 < file_length) {
				for (size_t i = position; i < file_length; i++) {
					if (isSpace(buffer[i])) {
						continue;
					}
					if ('<' == buffer[i] && '/' == buffer[i + 1]) {
						position = i;
					}
					else {
						position = parse(buffer, i, file_length, xml_node.children[key]);
					}
					break;
				}
				if ('<' == buffer[position] && '/' == buffer[position + 1]) {
					break;
				}
			}

			std::string key2;
			size_t position2 = file_length;
			if ('<' == buffer[position] && '/' == buffer[position + 1]) {
				for (size_t i = position + 2; i < file_length; i++) {
					if ('>' != buffer[i]) {
						key2 += buffer[i];
					}
					else {
						position2 = i + 1;
						break;
					}
				}
			}

			if (key != key2) {
				TriggerError(key + " and " + key2 + " doesn't match.");
			}

			for (size_t i = position2; i < file_length; i++) {
				if (!isSpace(buffer[i])) {
					return i;
				}
			}
			return file_length;
		}
	};
}
