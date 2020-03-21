#pragma once

#include <set>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "functions.hpp"

namespace MoonDb {

class CFileSystem
{
public:
	/**
	* 获取文件大小
	* @param filepath 文件路径
	* @return 返回文件大小（字节）
	*/
	inline static uint64_t FileSize(const std::string& filepath)
	{
		/*boost::system::error_code ec;
		uint64_t filesize = boost::filesystem::file_size(boost::filesystem::path(filepath), ec);
		if (ec.value())
		std::cout << ec.message() << std::endl;*/
		uint64_t filesize = boost::filesystem::file_size(boost::filesystem::path(filepath));
		return filesize;
	}

	/**
	* 获取文件最近更新时间
	* @param filepath 文件路径
	* @return 返回文件更新时间
	*/
	inline static std::time_t LastModifyTime(const std::string& filepath)
	{
		return boost::filesystem::last_write_time(boost::filesystem::path(filepath));
	}

	/**
	* 获取路径知否存在
	* @param filepath 路径（文件或目录）
	* @return 存在返回true，不存在返回false
	*/
	inline static bool Exists(const std::string& filepath)
	{
		return boost::filesystem::exists(boost::filesystem::path(filepath));
	}

	/**
	* 检查目录是否为空，或者文件大小是否为0
	* @param filepath 路径（文件或目录）
	* @return 是返回true，不是返回false
	*/
	inline static bool IsEmpty(const std::string& filepath)
	{
		return boost::filesystem::is_empty(boost::filesystem::path(filepath));
	}

	/**
	* 检查路径是否为目录
	* @param filepath 路径
	* @return 是返回true，不是返回false
	*/
	inline static bool IsDirectory(const std::string& filepath)
	{
		return boost::filesystem::is_directory(boost::filesystem::path(filepath));
	}

	/**
	* 检查路径是否为文件，比如名为VCS.tlog的目录也会被认为是文件，需要注意
	* @param filepath 路径
	* @return 是返回true，不是返回false
	*/
	inline static bool IsFile(const std::string& filepath)
	{
		return boost::filesystem::is_regular_file(boost::filesystem::path(filepath));
	}

	/**
	* 删除空目录或文件
	* @param filepath 路径（文件或目录）
	* @return 如果路径不存在返回false，否则返回true
	*/
	inline static bool RemoveFile(const std::string& filepath)
	{
		return boost::filesystem::remove(boost::filesystem::path(filepath));
	}

	/**
	* 递归删除目录或删除文件
	* @param filepath 路径（文件或目录）
	* @return 返回删除的文件数量
	*/
	inline static uint64_t RemoveDirectory(const std::string& filepath)
	{
		return boost::filesystem::remove_all(boost::filesystem::path(filepath));
	}

	/**
	* 创建目录
	* @param directory 目录路径
	* @return 创建成功返回true，失败返回false
	*/
	inline static bool CreateDirectory(const std::string& directory)
	{
		return boost::filesystem::create_directory(boost::filesystem::path(directory));
	}

	/**
	* 递归创建目录
	* @param directory 目录路径
	* @return 创建成功返回true，失败返回false
	*/
	inline static bool CreateDirectories(const std::string& directory)
	{
		return boost::filesystem::create_directories(boost::filesystem::path(directory));
	}

	/**
	* 拷贝文件
	* @param from 源文件路径
	* @param to 目的文件路径
	*/
	inline static void CopyFile(const std::string& from, const std::string& to)
	{
		return boost::filesystem::copy_file(boost::filesystem::path(from), boost::filesystem::path(to));
	}

	/**
	* 重命名文件或目录（目录下存在文件也可以重命名）
	* @param oldname 原来的路径
	* @param newname 新的路径
	*/
	inline static void RenameFile(const std::string& oldname, const std::string& newname)
	{
		return boost::filesystem::rename(boost::filesystem::path(oldname), boost::filesystem::path(newname));
	}

	/**
	* 读取目录（仅一级）
	* @param directory 目录路径
	* @param files 获取的子目录与文件
	* @param onlyfilename 为true时files中的文件只是文件名不包含路径，为false时为绝对路径
	* @return 如果directory是目录返回true，失败返回false
	*/
	inline static bool ReadDirectory(const std::string& directory, std::vector<std::string> & files, bool onlyfilename = false)
	{
		files.resize(0);
		if (IsDirectory(directory)) {
			boost::filesystem::directory_iterator end;
			for (boost::filesystem::directory_iterator pos(directory); pos != end; ++pos) {
				if (onlyfilename) {
					files.push_back(pos->path().filename().string());
				}
				else {
					files.push_back(pos->path().string());
				}
			}
			return true;
		}
		else
			return false;
	}

	/**
	* 读取目录中的文件（仅一级）
	* @param directory 目录路径
	* @param files 获取目录中的文件
	* @param onlyfilename 为true时files中的文件只是文件名不包含路径，为false时为绝对路径
	* @param extension 文件扩展名，如："png"；允许多个扩展名，如："jpeg|jpg|png|gif"
	* @return 如果directory是目录返回true，失败返回false
	*/
	inline static bool ReadDirectoryOnlyFile(const std::string& directory, std::vector<std::string> & files, bool onlyfilename = false, const std::string& extension = "")
	{
		files.resize(0);
		if (IsDirectory(directory)) {
			std::set<std::string> exts(explode_set(extension, '|'));
			boost::filesystem::directory_iterator end;
			for (boost::filesystem::directory_iterator pos(directory); pos != end; ++pos) {
				if (!boost::filesystem::is_directory(pos->path())) {
					if (extension.length() > 0) {
						std::string fileext = pos->path().extension().string();
						if (fileext.length() > 0)
							fileext = fileext.substr(1);
						if (exts.find(fileext) == exts.end()) {
							continue;
						}
					}
					if (onlyfilename) {
						files.push_back(pos->path().filename().string());
					}
					else {
						files.push_back(pos->path().string());
					}
				}
			}
			return true;
		}
		else
			return false;
	}

	/**
	* 读取目录中的目录（仅一级）
	* @param directory 目录路径
	* @param files 获取目录中的目录
	* @param onlyfilename 为true时files中的文件只是文件名不包含路径，为false时为绝对路径
	* @return 如果directory是目录返回true，失败返回false
	*/
	inline static bool ReadDirectoryOnlyDirectory(const std::string& directory, std::vector<std::string> & files, bool onlyfilename = false)
	{
		files.resize(0);
		if (IsDirectory(directory)) {
			boost::filesystem::directory_iterator end;
			for (boost::filesystem::directory_iterator pos(directory); pos != end; ++pos) {
				if (boost::filesystem::is_directory(pos->path())) {
					if (onlyfilename) {
						files.push_back(pos->path().filename().string());
					}
					else {
						files.push_back(pos->path().string());
					}
				}
			}
			return true;
		}
		else
			return false;
	}

	/**
	* 读取目录中的全部目录和文件
	* @param directory 目录路径
	* @param files 获取目录中的文件
	* @param onlyfilename 为true时files中的文件只是文件名不包含路径，为false时为绝对路径
	* @return 如果directory是目录返回true，失败返回false
	*/
	inline static bool ReadDirectoryAll(const std::string& directory, std::vector<std::string> & files, bool onlyfilename = false)
	{
		files.resize(0);
		if (IsDirectory(directory)) {
			boost::filesystem::recursive_directory_iterator end;
			for (boost::filesystem::recursive_directory_iterator pos(directory); pos != end; ++pos) {
				if (onlyfilename) {
					files.push_back(pos->path().filename().string());
				}
				else {
					files.push_back(pos->path().string());
				}
			}
			return true;
		}
		else
			return false;
	}

	/**
	* 读取目录中的全部文件，包括全部子目录
	* @param directory 目录路径
	* @param files 获取目录中的文件
	* @param onlyfilename 为true时files中的文件只是文件名不包含路径，为false时为绝对路径
	* @param extension 文件扩展名，如："png"；允许多个扩展名，如："jpeg|jpg|png|gif"
	* @return 如果directory是目录返回true，失败返回false
	*/
	inline static bool ReadDirectoryAllFiles(const std::string& directory, std::vector<std::string> & files, bool onlyfilename = false, const std::string& extension = "")
	{
		files.resize(0);
		if (IsDirectory(directory)) {
			std::set<std::string> exts(explode_set(extension, '|'));
			boost::filesystem::recursive_directory_iterator end;
			for (boost::filesystem::recursive_directory_iterator pos(directory); pos != end; ++pos) {
				if (!boost::filesystem::is_directory(pos->path())) {
					if (extension.length() > 0) {
						std::string fileext = pos->path().extension().string();
						if (fileext.length() > 0)
							fileext = fileext.substr(1);
						if (exts.find(fileext) == exts.end()) {
							continue;
						}
					}
					if (onlyfilename) {
						files.push_back(pos->path().filename().string());
					}
					else {
						files.push_back(pos->path().string());
					}
				}
			}
			return true;
		}
		else
			return false;
	}

	/**
	* 拷贝目录
	* @param from 源目录
	* @param to 目的目录
	* @param omitduplicate 如果为true那么忽略掉重复的文件，为false则强制覆盖
	*/
	inline static bool CopyDirectory(const std::string& from, const std::string& to, bool omitduplicate = false)
	{
		boost::filesystem::path from_dir(from);
		if (!boost::filesystem::is_directory(from_dir)) {
			return false;
		}

		boost::filesystem::path to_dir(to);
		if (!boost::filesystem::is_directory(to_dir) && !boost::filesystem::create_directories(to_dir)) {
			return false;
		}
		
		boost::filesystem::recursive_directory_iterator end;
		for (boost::filesystem::recursive_directory_iterator pos(from); pos != end; ++pos) {
			// 转换目录路径为目的路径
			boost::filesystem::path filepath = to_dir / pos->path().string().substr(from_dir.string().length());
			if (boost::filesystem::is_directory(pos->path())) {
				if (!boost::filesystem::is_directory(filepath)) {
					boost::filesystem::create_directory(filepath);
				}
			}
			else {
				// 如果文件已存在
				if (boost::filesystem::is_regular_file(filepath)) {
					// 覆盖文件
					if (!omitduplicate) {
						// 先删除以前的文件
						boost::filesystem::remove(filepath);
						// 再拷贝文件
						boost::filesystem::copy_file(pos->path(), filepath);
					}
				}
				// 如果文件不存在那么拷贝文件
				else {
					boost::filesystem::copy_file(pos->path(), filepath);
				}
			}
		}
		return true;
	}
};

}
