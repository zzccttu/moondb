#pragma once

#include <functional>
#include <chrono>
#include <random>

namespace MoonDb {

/**
* @brief   VRandom随机数类采用对象函数的方式编写。
* @version 1.0
* @author  mzt
*/

class CRandom
{
public:
	/**
	* 构造函数，用于产生随机数种子
	*/
	inline CRandom()
	{
		//generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
		generator.seed(static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	}

	/**
	* 生成随机字符串
	* @param length 字符串长度
	* @param min 最小ASCII值
	* @param max 最大ASCII值
	* @return 返回字符串
	*/
	inline std::string operator() (size_t length, int8_t min, int8_t max) {
		std::uniform_int_distribution<int8_t> distribution(min, max);
		// 绑定分布函数和发生器
		auto dice = std::bind(distribution, generator);
		// 初始化字符串
		std::string str(length, '\0');
		// 生成字符串
		for (size_t i = 0; i < length; i++) {
			// 为避免重复，抛弃数量一定的字符
			generator.discard(i % 4);
			// 生成字符
			str[i] = dice();
		}
		return str;
	}

	/**
	 * @brief 根据字符列表生成随机字符串
	 * @param length 随机字符串长度
	 * @param chars 随机字符列表
	 * @return 随机字符串
	 */
	inline std::string operator() (size_t length, const std::string& chars)
	{
		if(chars.size() == 0) {
			return "";
		}
		std::uniform_int_distribution<uint8_t> distribution(0, static_cast<uint8_t>(chars.size() - 1));
		auto dice = std::bind(distribution, generator);
		std::string str(length, '\0');
		for(size_t i = 0; i < length; i++) {
			generator.discard(i % 4);
			str[i] = chars[dice()];
		}
		return str;
	}

	/**
	* 生成随机数
	* @param min 最小值
	* @param max 最大值
	* @return 返回随机数
	*/
	inline int32_t operator() (int32_t min, int32_t max) {
		std::uniform_int_distribution<int32_t> distribution(min, max);
		return distribution(generator);
	}

	/**
	* 生成随机数
	* @param min 最小值
	* @param max 最大值
	* @return 返回随机数
	*/
	inline int64_t operator() (int64_t min, int64_t max) {
		std::uniform_int_distribution<int64_t> distribution(min, max);
		return distribution(generator);
	}

	/**
	 * @brief 生成128位无符号随机数
	 * @param ipnum 丢弃的基数，如果用来生成session，可以传入ip
	 * @return 返回随机数
	 */
	inline __uint128_t operator() (__uint128_t ipnum) {
		uint8_t discardnum = 0;
		uint8_t* p = static_cast<uint8_t*>(static_cast<void*>(&ipnum));
		for(uint8_t i = 0; i < 16; i++) {
			discardnum += *(p + i);
		}
		std::uniform_int_distribution<uint64_t> distribution(0, std::numeric_limits<uint64_t>::max());
		auto dice = std::bind(distribution, generator);
		generator.discard(discardnum % 3);
		__uint128_t num = __uint128_t(dice()) << 64;
		generator.discard(discardnum % 5);
		num += dice();
		return num;
	}

	/**
	* 生成随机数
	* @param min 最小值
	* @param max 最大值
	* @return 返回随机数
	*/
	inline float operator() (float min, float max) {
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(generator);
	}

	/**
	* 生成随机数
	* @param min 最小值
	* @param max 最大值
	* @return 返回随机数
	*/
	inline double operator() (double min, double max) {
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(generator);
	}

protected:
	std::default_random_engine generator;	/**< 随机数发生器 */
};

}
