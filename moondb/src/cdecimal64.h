#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <map>
#include <quadmath.h>
#include "crunningerror.hpp"
#include "functions.hpp"

namespace MoonDb {

class CDecimal64
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

	CDecimal64()
	{
		Scale = NotScale;
		Data = 0;
	}

	CDecimal64(ScaleType scale)
	{
		Scale = scale;
		Data = 0;
	}

	CDecimal64(const std::string& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const int64_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const uint64_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const int32_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const uint32_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const int16_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const uint16_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const int8_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const uint8_t& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const float& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const double& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const __float128& num, ScaleType scale)
	{
		Scale = scale;
		Set(num);
	}

	CDecimal64(const CDecimal64& num, ScaleType scale)
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
	void Set(const int64_t& num);
	void Set(const uint64_t& num);

	inline void Set(const CDecimal64& num)
	{
		if(num.GetScale() == Scale) {
			Data = num.GetData();
		}
		else {
			Data = num.ChangeScale(Scale);
		}
	}

	inline void Set(const int32_t& num)
	{
		Set(static_cast<int64_t>(num));
	}

	inline void Set(const uint32_t& num)
	{
		Set(static_cast<uint64_t>(num));
	}

	inline void Set(const int16_t& num)
	{
		Set(static_cast<int64_t>(num));
	}

	inline void Set(const uint16_t& num)
	{
		Set(static_cast<uint64_t>(num));
	}

	inline void Set(const int8_t& num)
	{
		Set(static_cast<int64_t>(num));
	}

	inline void Set(const uint8_t& num)
	{
		Set(static_cast<uint64_t>(num));
	}

	void Set(const float& num);
	void Set(const double& num);
	void Set(const __float128& num);

	// 返回转换精度后的Data值
	int64_t ChangeScale(ScaleType newscale) const;

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

	inline friend std::ostream & operator << (std::ostream & os, const CDecimal64& dec)
	{
		os << dec.ToString();
		return os;
	}

	/*inline CDecimal64& operator= (const CDecimal64& num)
	{
		this->Data = num.GetData();
		this->Scale = num.GetScale();
		return *this;
	}*/

	inline CDecimal64& operator= (const std::string& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const int64_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const uint64_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const int32_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const uint32_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const int16_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const uint16_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const int8_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const uint8_t& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const float& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const double& num)
	{
		Set(num);
		return *this;
	}

	inline CDecimal64& operator= (const __float128& num)
	{
		Set(num);
		return *this;
	}

	// 转换返回指定类型
	int64_t ToInt64(RoundType rt = RT_ROUND, bool throwexception = true);
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

	inline friend CDecimal64 operator-(const CDecimal64 &A)
	{
		CDecimal64 B;
		B.Data = -A.Data;
		B.Scale = A.Scale;
		return B;
	}

	inline friend bool operator!(const CDecimal64 &A)
	{
		return A.GetData() == 0;
	}

	inline friend bool operator==(const CDecimal64 &A, const CDecimal64 &B)
	{
		return A.ChangeScale(B.GetScale()) == B.GetData();
	}

	inline friend bool operator!=(const CDecimal64 &A, const CDecimal64 &B)
	{
		return A.ChangeScale(B.GetScale()) != B.GetData();
	}

	friend CDecimal64 operator+(const CDecimal64 &A, const CDecimal64 &B);

	inline friend CDecimal64 operator-(const CDecimal64 &A, const CDecimal64 &B)
	{
		return A + -B;
	}

	friend CDecimal64 operator*(const CDecimal64 &A, const CDecimal64 &B);

	friend CDecimal64 operator/(const CDecimal64 &A, const CDecimal64 &B);

	inline CDecimal64 & operator+=(const CDecimal64 &B)
	{
		*this = *this + B;
		return *this;
	}

	inline CDecimal64 & operator-=(const CDecimal64 &B)
	{
		*this = *this + -B;
		return *this;
	}

	inline CDecimal64 & operator*=(const CDecimal64 &B)
	{
		*this = *this * B;
		return *this;
	}

	inline CDecimal64 & operator/=(const CDecimal64 &B)
	{
		*this = *this / B;
		return *this;
	}

	void MakeScaleEffective();

	inline static const CDecimal64& One()
	{
		static CDecimal64 val(1, 0);
		return val;
	}

	inline static const CDecimal64& Two()
	{
		static CDecimal64 val(2, 0);
		return val;
	}

protected:
	ScaleType Scale;	/**< -32767至32767，如果大于0为小数点后的数字个数，小于0为小数点前的0的个数，为0表示无小数从个位数字开始 */
	int64_t Data;	/**< 将数值存储在这 */
	const static ScaleType MinScale;		/**< Scale最小值 */
	const static ScaleType MaxScale;		/**< Scale最大值 */
	const static DigitsType MaxScaleLength;	/**< 指数最大长度 */
	const static DigitsType MaxPrecision;	/**< 64位整数的数字个数为39个，第一个数字为9的数字个数为38 */
	const static std::string MaxValueString;/**< 64位整数最大值字符串形式 */
	const static std::string MinValueString;/**< 64位整数最小值字符串形式 */
	const static int64_t MaxValueInt;		/**< 64位整数最大值 */
	const static int64_t MinValueInt;		/**< 64位整数最小值 */
	const static std::string DataRangeString;

	// 在乘以10之前检测是否会溢出
	inline bool IfDataMul10Invalid(const int64_t& v) const noexcept
	{
		return v > 922337203685477580LL || v < -9223372036854775807LL;
	}

	inline static int64_t pow_10(const ScaleType& exponent)
	{
		/*static std::vector<int64_t> values;
		if(values.size() == 0) {
			int64_t mul = 1;
			values.push_back(mul);
			for(ScaleType i = 1; i <= MaxPrecision; i++) {
				mul *= 10;
				values.push_back(mul);
			}
		}
		return values[exponent];*/
		switch(exponent) {
			case 0:
				return 1;
			case 1:
				return 10;
			case 2:
				return 100;
			case 3:
				return 1000;
			case 4:
				return 10000;
			case 5:
				return 100000;
			case 6:
				return 1000000;
			case 7:
				return 10000000;
			case 8:
				return 100000000;
			case 9:
				return 1000000000;
			case 10:
				return 10000000000;
			case 11:
				return 100000000000;
			case 12:
				return 1000000000000;
			case 13:
				return 10000000000000;
			case 14:
				return 100000000000000;
			case 15:
				return 1000000000000000;
			case 16:
				return 10000000000000000;
			case 17:
				return 100000000000000000;
			case 18:
				return 1000000000000000000;
			default:
				return 0;
		}
	}

	inline static __int128_t pow_10_128(const ScaleType& exponent)
	{
		static std::vector<__int128_t> values;
		if(values.size() == 0) {
			__int128_t mul = 1;
			values.push_back(mul);
			for(ScaleType i = 1; i <= 38; i++) {
				mul *= 10;
				values.push_back(mul);
			}
		}
		return values[exponent];
	}

	inline static DigitsType GetDigits(const int64_t& data)
	{
		/*static std::map<int64_t, DigitsType> ranges;
		if(ranges.size() == 0) {
			int64_t k = 1;
			for(DigitsType i = 0; i < std::numeric_limits<int64_t>::digits10; i++) {
				ranges[k] = i + 1;
				k *= 10;
			}
		}
		int64_t d = data >= 0 ? data : -data;
		// 反向遍历
		for(auto it = ranges.rbegin(); it != ranges.rend(); it++) {
			if(d >= it->first) {
				return it->second;
			}
		}
		return 1;*/
		int64_t d = data >= 0 ? data : -data;
		if(d < 10) {
			return 1;
		}
		else if(d < 100) {
			return 2;
		}
		else if(d < 1000) {
			return 3;
		}
		else if(d < 10000) {
			return 4;
		}
		else if(d < 100000) {
			return 5;
		}
		else if(d < 1000000) {
			return 6;
		}
		else if(d < 10000000) {
			return 7;
		}
		else if(d < 100000000) {
			return 8;
		}
		else if(d < 1000000000) {
			return 9;
		}
		else if(d < 10000000000) {
			return 10;
		}
		else if(d < 100000000000) {
			return 11;
		}
		else if(d < 1000000000000) {
			return 12;
		}
		else if(d < 10000000000000) {
			return 13;
		}
		else if(d < 100000000000000) {
			return 14;
		}
		else if(d < 1000000000000000) {
			return 15;
		}
		else if(d < 10000000000000000) {
			return 16;
		}
		else if(d < 100000000000000000) {
			return 17;
		}
		else if(d < 1000000000000000000) {
			return 18;
		}
		else {
			return 19;
		}
	}

//	inline static DigitsType GetDigits(const __int128_t& data)
//	{
//		static std::map<__int128_t, DigitsType> ranges;
//		if(ranges.size() == 0) {
//			__int128_t k = 1;
//			for(DigitsType i = 0; i < 39; i++) {
//				ranges[k] = i + 1;
//				k *= 10;
//			}
//		}
//		__int128_t d = data >= 0 ? data : -data;
//		// 反向遍历
//		for(auto it = ranges.rbegin(); it != ranges.rend(); it++) {
//			if(d >= it->first) {
//				return it->second;
//			}
//		}
//		return 1;
//	}

	inline static DigitsType ToMaxDigits(const int64_t& data)
	{
		/*static std::map<int64_t, DigitsType> multimes;
		if(multimes.size() == 0) {
			int64_t k = MaxValueInt;
			for(DigitsType i = 0; i < MaxPrecision; i++) {
				k /= 10;
				multimes[k] = i + 1;
			}
		}
		int64_t d = data >= 0 ? data : -data;
		for(auto it = multimes.begin(); it != multimes.end(); it++) {
			if(d <= it->first) {
				return it->second;
			}
		}
		return 0;*/
		int64_t d = data >= 0 ? data : -data;
		if(d <= 9) {
			return 18;
		}
		else if(d <= 92) {
			return 17;
		}
		else if(d <= 922) {
			return 16;
		}
		else if(d <= 9223) {
			return 15;
		}
		else if(d <= 92233) {
			return 14;
		}
		else if(d <= 922337) {
			return 13;
		}
		else if(d <= 9223372) {
			return 12;
		}
		else if(d <= 92233720) {
			return 11;
		}
		else if(d <= 922337203) {
			return 10;
		}
		else if(d <= 9223372036) {
			return 9;
		}
		else if(d <= 92233720368) {
			return 8;
		}
		else if(d <= 922337203685) {
			return 7;
		}
		else if(d <= 9223372036854) {
			return 6;
		}
		else if(d <= 92233720368547) {
			return 5;
		}
		else if(d <= 922337203685477) {
			return 4;
		}
		else if(d <= 9223372036854775) {
			return 3;
		}
		else if(d <= 92233720368547758) {
			return 2;
		}
		else if(d <= 922337203685477580) {
			return 1;
		}
		else {
			return 0;
		}
	}

	// 如果超出范围返回true，正常返回false
	inline bool CheckIfIntStringOutOfRange(const std::string& numstr)
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
		return "Number '" + num + "' is out of range (from " + num_to_string(MaxValueInt) + "E" + num_to_string(scale) + " to " + num_to_string(MinValueInt) + "E" + num_to_string(scale) + ")";
	}

	inline std::string LimitString(const int64_t& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num_to_string(num) + "' is out of range (from " + num_to_string(MaxValueInt) + "E" + num_to_string(scale) + " to " + num_to_string(MinValueInt) + "E" + num_to_string(scale) + ")";
	}

	inline std::string LimitString(const uint64_t& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num_to_string(num) + "' is out of range (from " + num_to_string(MaxValueInt) + "E" + num_to_string(scale) + " to " + num_to_string(MinValueInt) + "E" + num_to_string(scale) + ")";
	}

	inline std::string LimitString(const __float128& num)
	{
		ScaleType scale = Scale;
		if(NotScale == scale)
			scale = MaxScale;
		return "Number '" + num_to_string(num) + "' is out of range (from " + num_to_string(MaxValueInt) + "E" + num_to_string(scale) + " to " + num_to_string(MinValueInt) + "E" + num_to_string(scale) + ")";
	}

	inline std::string LimitString2(const __float128& num)
	{
		static std::stringstream ss;
		if(ss.str().size() == 0) {
			ss << "' is out of range (from " << -FLT128_MAX << " to " << FLT128_MAX << ")";
		}
		return "Number '" + num_to_string(num) + ss.str() + " when the scale is " + std::to_string(Scale);
	}

	static bool Plus(const int64_t& a_data, const int64_t& b_data, int64_t& result, ScaleType& scale);
	static void Shrink(__int128_t& result, ScaleType& scale);
};
}
