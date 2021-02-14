#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <map>
#include "crunningerror.hpp"
#include "functions.hpp"
#include "gmp/gmp.h"

#include <boost/multiprecision/float128.hpp>
using boost::multiprecision::float128;
#include <boost/multiprecision/cpp_dec_float.hpp>
using namespace boost::multiprecision;
#include <boost/math/constants/constants.hpp>

namespace MoonDb {

class CDecimal128
{
public:
	typedef int32_t ScaleType;				/**< Scale类型 */
	typedef int32_t DigitsType;				/**< 数字长度，用于有效数字和指数数字的个数 */
	const static ScaleType NotScale;		/**< 非Scale值 */

	enum RoundType {
		RT_ROUND = 1,
		RT_CEIL,
		RT_FLOOR
	};

	CDecimal128()
	{
		Scale = NotScale;
		Data = 0;
	}

	CDecimal128(ScaleType scale)
	{
		Scale = scale;
		Data = 0;
	}

	CDecimal128(const std::string& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const __int128_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const __uint128_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const int64_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const uint64_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const int32_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const uint32_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const int16_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const uint16_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const int8_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const uint8_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const float& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const double& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const __float128& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal128(const CDecimal128& num, ScaleType scale)
	{
		if(NotScale == scale) {
			Scale = num.GetScale();
			Data = num.GetData();
		}
		else {
			Scale = scale;
			Set(num);
		}
	}

	void Set(const std::string& num);
	void Set(const __int128_t& num);
	void Set(const __uint128_t& num);

	inline void Set(const CDecimal128& num)
	{
		if(num.GetScale() == Scale) {
			Data = num.GetData();
		}
		else {
			Data = num.ChangeScale(Scale);
		}
	}

	inline void Set(const int64_t& num)
	{
		Set(static_cast<__int128_t>(num));
	}

	inline void Set(const uint64_t& num)
	{
		Set(static_cast<__uint128_t>(num));
	}

	inline void Set(const int32_t& num)
	{
		Set(static_cast<__int128_t>(num));
	}

	inline void Set(const uint32_t& num)
	{
		Set(static_cast<__uint128_t>(num));
	}

	inline void Set(const int16_t& num)
	{
		Set(static_cast<__int128_t>(num));
	}

	inline void Set(const uint16_t& num)
	{
		Set(static_cast<__uint128_t>(num));
	}

	inline void Set(const int8_t& num)
	{
		Set(static_cast<__int128_t>(num));
	}

	inline void Set(const uint8_t& num)
	{
		Set(static_cast<__uint128_t>(num));
	}

	void Set(const float& num);
	void Set(const double& num);
	void Set(const __float128& num);

	// 返回转换精度后的Data值
	__int128_t ChangeScale(ScaleType newscale) const;

	// trimzero为true表示去掉小数点后的用于对齐的多余的0，为false保留多余的0比如4为小数点后的位数123.4500
	// roundprec设置四舍五入的小数点位置，为正表示小数点后的位数，为负表示小数点前的位数，为0表示整数，-32768表示不进行四舍五入
	// iffixed为true表示返回纯小数，为false表示返回科学计数法形式的数
	std::string ToString(bool iffixed = true, bool trimzero = false, ScaleType roundprec = NotScale) const noexcept;

	// 获取数值以便存储
	inline __int128_t GetData() const noexcept
	{
		return Data;
	}

	// 设置数值便于处理，比如转换为字符串
	inline void SetData(const __int128_t& data = 0) noexcept
	{
		Data = data;
	}

	inline ScaleType GetScale() const noexcept
	{
		return Scale;
	}

	inline void SetScale(const ScaleType& scale = NotScale) noexcept
	{
		Scale = scale;
	}

	inline friend std::ostream & operator << (std::ostream & os, const CDecimal128& dec)
	{
		os << dec.ToString();
		return os;
	}

	/*inline CDecimal128& operator= (const CDecimal128& num)
	{
		this->Data = num.GetData();
		this->Scale = num.GetScale();
		return *this;
	}*/

	inline CDecimal128& operator= (const std::string& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const __int128_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const __uint128_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const int64_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const uint64_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const int32_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const uint32_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const int16_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const uint16_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const int8_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const uint8_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const float& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const double& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal128& operator= (const __float128& num)
	{
		Set(num);
		return *this;
	}

	// 转换返回指定类型
	__int128_t ToInt128(RoundType rt = RT_ROUND, bool throwexception = true);
	__uint128_t ToUInt128(RoundType rt = RT_ROUND);
	int64_t ToInt64(RoundType rt = RT_ROUND);
	uint64_t ToUInt64(RoundType rt = RT_ROUND);
	int32_t ToInt32(RoundType rt = RT_ROUND);
	uint32_t ToUInt32(RoundType rt = RT_ROUND);
	int16_t ToInt16(RoundType rt = RT_ROUND);
	uint16_t ToUInt16(RoundType rt = RT_ROUND);
	int8_t ToInt8(RoundType rt = RT_ROUND);
	uint8_t ToUInt8(RoundType rt = RT_ROUND);
	float ToFloat32();
	double ToFloat64();
	__float128 ToFloat128();

	// 改变符号的操作符，因为该类中将-170141183460469231731687303715884105728舍掉，所以取负值之后不会出现溢出
	inline friend CDecimal128 operator-(const CDecimal128 &A)
	{
		CDecimal128 B;
		B.Data = -A.Data;
		B.Scale = A.Scale;
		return B;
	}

	inline friend bool operator!(const CDecimal128 &A)
	{
		return A.Data == 0;
	}

	inline friend bool operator==(const CDecimal128 &A, const CDecimal128 &B)
	{
		return A.ChangeScale(B.Scale) == B.Data;
	}

	inline friend bool operator!=(const CDecimal128 &A, const CDecimal128 &B)
	{
		return A.ChangeScale(B.Scale) != B.Data;
	}

	friend CDecimal128 operator+(const CDecimal128 &A, const CDecimal128 &B);

	inline friend CDecimal128 operator-(const CDecimal128 &A, const CDecimal128 &B)
	{
		return A + -B;
	}

	friend CDecimal128 operator*(const CDecimal128 &A, const CDecimal128 &B);

	friend CDecimal128 operator/(const CDecimal128 &A, const CDecimal128 &B);

	CDecimal128 & operator+=(const CDecimal128 &B);

	inline CDecimal128 & operator-=(const CDecimal128 &B)
	{
		*this += -B;
		return *this;
	}

	inline CDecimal128 & operator*=(const CDecimal128 &B)
	{
		*this = *this * B;
		return *this;
	}

	inline CDecimal128 & operator/=(const CDecimal128 &B)
	{
		*this = *this / B;
		return *this;
	}

	inline static const CDecimal128& One()
	{
		static CDecimal128 val(1, 0);
		return val;
	}

	inline static const CDecimal128& Two()
	{
		static CDecimal128 val(2, 0);
		return val;
	}

protected:
	ScaleType Scale;	/**< -32767至32767，如果大于0为小数点后的数字个数，小于0为小数点前的0的个数，为0表示无小数从个位数字开始 */
	__int128_t Data;	/**< 将数值存储在这里 */
	const static ScaleType MinScale;		/**< Scale最小值 */
	const static ScaleType MaxScale;		/**< Scale最大值 */
	const static DigitsType MaxScaleLength;	/**< 指数最大长度 */
	const static DigitsType MaxPrecision;	/**< 128位整数的数字个数为39个，第一个数字为9的数字个数为38 */
	const static DigitsType MaxPrecisionU;	/**< 128位无符号整数的数字个数，为39个 */
	const static __int128_t MaxValueInt;	/**< 128位整数最大值 */
	const static __int128_t MinValueInt;	/**< 128位整数最小值 */
	const static std::string MaxValueString;/**< 128位整数最大值字符串形式 */
	const static std::string MinValueString;/**< 128位整数最小值字符串形式 */
	const static std::string DataRangeString;/**< 范围字符串 */
	const static __int128_t MaxDivInt;		/**< 最大除数 */
	const static __int128_t MinDivInt;		/**< 最小除数 */
	const static __uint128_t TwoPow64;		/**< 2^64 */

	union DataType {
		__uint128_t d128;
		// 0低位，1高位
		uint64_t d64[2];
	};

	// 如果超出范围返回true，正常返回false
	inline static bool CheckIfIntStringOutOfRange(const std::string& numstr)
	{
		size_t len = numstr.size();
		if('-' == numstr[0]) {
			for(size_t i = 1; i < len; i++) {
				if(numstr[i] > MinValueString[i]) {
					return true;
				}
				else if(numstr[i] < MinValueString[i]) {
					return false;
				}
			}
		}
		else {
			for(size_t i = 0; i < len; i++) {
				if(numstr[i] > MaxValueString[i]) {
					return true;
				}
				else if(numstr[i] < MaxValueString[i]) {
					return false;
				}
			}
		}
		return false;
	}

	inline std::string LimitString(const std::string& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num + "' is out of range (" + DataRangeString + ")";
	}

	inline std::string LimitString(const __int128_t& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num_to_string(num) + "' is out of range (" + DataRangeString + ")";
	}

	inline std::string LimitString(const __uint128_t& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num_to_string(num) + "' is out of range (" + DataRangeString + ")";
	}

	inline std::string LimitString(const __float128& num)
	{
		static std::stringstream ss;
		if(ss.str().size() == 0) {
			ss << "' is out of range (from " << -FLT128_MAX << " to " << FLT128_MAX << ")";
		}
		return "Number '" + num_to_string(num) + ss.str() + " when the scale is " + std::to_string(Scale);
	}

	static void SetGMPInt(mpz_t mpz, const __uint128_t& data);

	static bool Plus(const __int128_t& a_data, const __int128_t& b_data, __int128_t& result, ScaleType& scale);

	inline static DigitsType ToMaxDigits(const __int128_t& data)
	{
		static std::map<__int128_t, DigitsType> multimes;
		if(multimes.size() == 0) {
			__int128_t k = MaxValueInt;
			for(DigitsType i = 0; i < MaxPrecision; i++) {
				k /= 10;
				multimes[k] = i + 1;
			}
		}
		__int128_t d = data >= 0 ? data : -data;
		for(auto it = multimes.begin(); it != multimes.end(); it++) {
			if(d <= it->first) {
				return it->second;
			}
		}
		return 0;
	}

	inline static DigitsType GetDigits(const __int128_t& data)
	{
		static std::map<__int128_t, DigitsType> ranges;
		if(ranges.size() == 0) {
			__int128_t k = 1;
			for(DigitsType i = 0; i < MaxPrecision; i++) {
				ranges[k] = i + 1;
				k *= 10;
			}
		}
		__int128_t d = data >= 0 ? data : -data;
		// 反向遍历
		for(auto it = ranges.rbegin(); it != ranges.rend(); it++) {
			if(d >= it->first) {
				return it->second;
			}
		}
		return 1;
	}

	inline static DigitsType GetDigits(const __uint128_t& data)
	{
		static std::map<__uint128_t, DigitsType> ranges;
		if(ranges.size() == 0) {
			__uint128_t k = 1;
			for(DigitsType i = 0; i < MaxPrecision; i++) {
				ranges[k] = i + 1;
				k *= 10;
			}
		}
		// 反向遍历
		for(auto it = ranges.rbegin(); it != ranges.rend(); it++) {
			if(data >= it->first) {
				return it->second;
			}
		}
		return 1;
	}

	inline static DigitsType GetDigits(const uint64_t& data)
	{
		static std::map<uint64_t, DigitsType> ranges;
		if(ranges.size() == 0) {
			uint64_t k = 1;
			for(DigitsType i = 0; i < std::numeric_limits<uint64_t>::digits10; i++) {
				ranges[k] = i + 1;
				k *= 10;
			}
		}
		// 反向遍历
		for(auto it = ranges.rbegin(); it != ranges.rend(); it++) {
			if(data >= it->first) {
				return it->second;
			}
		}
		return 1;
	}

	inline static __int128_t pow_10(const ScaleType& exponent)
	{
		static std::vector<__int128_t> values;
		if(values.size() == 0) {
			__int128_t mul = 1;
			values.push_back(mul);
			for(ScaleType i = 1; i <= MaxPrecision; i++) {
				mul *= 10;
				values.push_back(mul);
			}
		}
		return values[exponent];
	}

	static void gmp_int_div_10(mpz_t d, ScaleType e)
	{
		ScaleType times = e / 9;
		ScaleType r = e % 9;
		for(ScaleType i = 0; i < times; i++) {
			mpz_tdiv_q_ui(d, d, pow_10(9));
		}
		if(r > 0) {
			mpz_tdiv_q_ui(d, d, pow_10(r));
		}
	}

	static bool ChangeScaleForMultiplication(__uint128_t& data, ScaleType oldscale, ScaleType newscale);
	static bool Plus(__int128_t& a_data, const __int128_t& b_data, ScaleType& scale);

	// 取得小数部分和指数，比如12.345取出后mantissa=1.2345、exponent=1
	void Extract(__float128& mantissa, ScaleType& exponent) const;

	// 取倒数
	CDecimal128 Inverse() const;

	// 放大10的muldigits次方倍，muldigits最大为18
	//static void MagnifyData(CDecimal128::DigitsType muldigits, const CDecimal128::DataType (&src)[2], CDecimal128::DataType (&des)[2]);
};
}
