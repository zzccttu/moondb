#include "cdecimal64.h"

namespace MoonDb {

const CDecimal64::ScaleType CDecimal64::NotScale = -32768;
const CDecimal64::ScaleType CDecimal64::MinScale = -32747;
const CDecimal64::ScaleType CDecimal64::MaxScale = 32729;
const CDecimal64::DigitsType CDecimal64::MaxScaleLength = 18;
const CDecimal64::DigitsType CDecimal64::MaxPrecision = 19;
const std::string CDecimal64::MaxValueString = "9223372036854775807";
const std::string CDecimal64::MinValueString = "-9223372036854775807";
const int64_t CDecimal64::MaxValueInt = 9223372036854775807LL;
const int64_t CDecimal64::MinValueInt = -9223372036854775807LL;
const std::string CDecimal64::DataRangeString = "from -9.223372036854775808E32747 to 9.223372036854775807E32747";

void CDecimal64::Set(const std::string& num)
{
	if(num.size() == 0) {
		Data = 0;
		return;
	}

	bool isnegative = '-' == num[0];
	// 将指数前的小数转换为整数
	std::string integer_str, fraction_str, exponent_str;
	integer_str.reserve(40);
	fraction_str.reserve(40);
	exponent_str.reserve(40);
	const char* p = &num.front();
	if(isnegative) {
		p++;
		integer_str.push_back('-');
	}
	while('0' == *p) {
		p++;
	}
	std::string *recordstr = &integer_str;
	while(*p) {
		switch(*p) {
		case '.':
			recordstr = &fraction_str;
			break;
		case 'e':
		case 'E':
			recordstr = &exponent_str;
			break;
		default:
			recordstr->push_back(*p);
		}
		p++;
	}
	// 指数
	int64_t exponent = 0;
	if(exponent_str.size() > 0) {
		if('-' == exponent_str[0]) {
			exponent_str = "-" + ltrim_copy(exponent_str.substr(1), '0');
			if(exponent_str.size() > MaxScaleLength + 1) {
				Data = 0;
				return;
			}
		}
		else {
			exponent_str = ltrim_copy(exponent_str, '0');
			if(exponent_str.size() > MaxScaleLength) {
				ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
			}
		}
		if(exponent_str.size() > 0) {
			exponent = stoll(exponent_str);
		}
	}
	std::string numint;
	// 如果存在小数部分
	if(fraction_str.size() > 0) {
		if('0' == fraction_str[fraction_str.size() - 1]) {
			int64_t i = static_cast<int64_t>(fraction_str.size()) - 2;
			for(; i >= 0; i--) {
				if('0' != fraction_str[i]) {
					break;
				}
			}
			fraction_str = fraction_str.substr(0, i + 1);
		}
		exponent -= fraction_str.size();
		numint = integer_str + fraction_str;
	}
	// 如果没有小数部分
	else {
		numint = integer_str;
	}

	int64_t numintsize = static_cast<int64_t>(numint.size());
	// 如果Scale未知，在设置时初始设置
	if(NotScale == Scale) {
		int64_t se = exponent;
		char lastdigit = '0';
		// 负数
		if(isnegative) {
			if(numintsize > MaxPrecision + 1) {
				numintsize = MaxPrecision + 1;
				se += numint.size() - numintsize;
				lastdigit = numint[numintsize];
				numint = numint.substr(0, numintsize);
			}
			if(MaxPrecision + 1 == numintsize && CheckIfIntStringOutOfRange(numint)) {
				numintsize = MaxPrecision;
				se += numint.size() - numintsize;
				lastdigit = numint[numintsize];
				numint = numint.substr(0, numintsize);
			}
		}
		//正数
		else {
			if(numintsize > MaxPrecision) {
				numintsize = MaxPrecision;
				se += numint.size() - numintsize;
				lastdigit = numint[numintsize];
				numint = numint.substr(0, numintsize);
			}
			if(MaxPrecision == numintsize && CheckIfIntStringOutOfRange(numint)) {
				numintsize = MaxPrecision - 1;
				se += numint.size() - numintsize;
				lastdigit = numint[numintsize];
				numint = numint.substr(0, numintsize);
			}
		}
		int64_t scaleexp = se + numintsize - (isnegative ? 1 : 0);
		if(scaleexp > MaxScale + MaxPrecision) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
		else if(scaleexp < MinScale) {
			Scale = 0;
			Data = 0;
		}
		else if(MinScale == scaleexp) {
			if(numint[(isnegative ? 1 : 0)] >= '5') {
				Scale = scaleexp;
				if(isnegative) {
					Data = -1;
				}
				else {
					Data = 1;
				}
			}
			else {
				Scale = 0;
				Data = 0;
			}
		}
		else {
			Scale = se;
			Data = stolll(numint);
			// 如果已达到最大或最小值就不再进一，而是将其舍掉，否则如果提高Scale的值会导致精度的进一步丢失
			if(lastdigit >= '5') {
				if(isnegative && Data > MinValueInt) {
					Data--;
				}
				else if(!isnegative && Data < MaxValueInt) {
					Data++;
				}
			}
		}
	}
	else {
		DigitsType maxpreclen = MaxPrecision + (isnegative ? 1 : 0);
		char lastdigit = '0';
		// 整数部分长度大于MaxPrecision
		if(numintsize > static_cast<int64_t>(maxpreclen)) {
			exponent += numintsize - maxpreclen;
			numintsize = maxpreclen;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
		if(numintsize == static_cast<int64_t>(maxpreclen) && CheckIfIntStringOutOfRange(numint)) {
			exponent++;
			numintsize--;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
		int64_t nn = static_cast<int64_t>(numintsize) - (isnegative ? 1 : 0) + exponent;
		if(nn > MaxPrecision + Scale) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
		// 如果数值过小，数据置为0
		else if(nn < Scale) {
			Data = 0;
		}
		else {
			Data = stolll(numint);
			// 考虑到可能导致更大的精度损失，如果已经为最大或最小值则不再进一
			if(lastdigit >= '5') {
				if(isnegative && Data > MinValueInt) {
					Data--;
				}
				else if(!isnegative && Data < MaxValueInt) {
					Data++;
				}
			}
			// 调整Data使得指数到Scale值
			if(exponent < Scale) {
				if(Scale - exponent > MaxPrecision) {
					Data = 0;
				}
				else {
					Data /= pow_10(Scale - 1 - exponent);
					int8_t lastdigit = Data % 10;
					Data /= 10;
					if(lastdigit >= 5) {
						Data++;
					}
					else if(lastdigit <= -5) {
						Data--;
					}
				}
			}
			else if(exponent > Scale) {
				ScaleType multimes = ToMaxDigits(Data);
				if(exponent - Scale > multimes) {
					ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
				}
				Data *= pow_10(exponent - Scale);
			}
		}
	}
}

void CDecimal64::Set(const int64_t& num)
{
	if(NotScale == Scale) {
		Scale = 0;
	}
	if(0 == num) {
		Data = 0;
	}
	else if(0 == Scale) {
		Data = num;
		if(num_limits<int64_t>::min() == Data) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
	}
	else if(Scale < 0) {
		Data = num;
		ScaleType multimes = ToMaxDigits(Data);
		if(-Scale > multimes) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
		Data *= pow_10(-Scale);
	}
	else {
		if(Scale > MaxPrecision) {
			Data = 0;
		}
		else {
			Data = num / pow_10(Scale - 1);
			int8_t lastdigit = Data % 10;
			Data /= 10;
			if(lastdigit >= 5) {
				Data++;
			}
			else if(lastdigit <= -5) {
				Data--;
			}
		}
	}
}

void CDecimal64::Set(const uint64_t& num)
{
	if(NotScale == Scale) {
		Scale = 0;
	}
	Data = num;
	if(0 == Data) {
		return;
	}
	if(Data < 0) {
		if(Scale <= 0) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
	}
	if(Scale <= 0) {
		ScaleType multimes = ToMaxDigits(Data);
		if(-Scale > multimes) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
		Data *= pow_10(-Scale);
	}
	else if(Scale > 0) {
		if(Scale > MaxPrecision) {
			Data = 0;
		}
		else {
			if(Data > 0) {
				Data /= pow_10(Scale - 1);
				int8_t lastdigit = Data % 10;
				Data /= 10;
				if(lastdigit >= 5) {
					Data++;
				}
			}
			else {
				uint64_t n = num;
				n /= pow_10(Scale - 1);
				int8_t lastdigit = n % 10;
				n /= 10;
				if(lastdigit >= 5) {
					n++;
				}
				Data = n;
			}
		}
	}
}

std::string CDecimal64::ToString(bool iffixed, bool trimzero, ScaleType roundprec) const noexcept
{
	int64_t data = Data;
	bool ifround = roundprec != NotScale && roundprec < -Scale;
	if(ifround) {
		if(-Scale - roundprec > MaxPrecision) {
			data = 0;
		}
		else {
			int64_t mul = 1;
			for(ScaleType i = roundprec; i < -Scale - 1; i++) {
				mul *= 10;
			}
			data /= mul;
			if(data != 0) {
				int8_t lastdigit = data % 10;
				data /= 10;
				if(lastdigit >= 5) {
					data++;
				}
				else if(lastdigit <= -5) {
					data--;
				}
				data *= mul;
				data *= 10;
			}
		}
	}
	std::string str = num_to_string(data);
	if(iffixed) {
		if(0 == Scale) {
			return str;
		}
		else if(Scale < 0) {
			// str长度最小为Scale
			if(str[0] != '-' && str.size() <= static_cast<size_t>(-Scale)) {
				str = "0." + std::string(static_cast<size_t>(-Scale) - str.size(), '0') + str;
			}
			else if(str[0] == '-' && str.size() <= static_cast<size_t>(-Scale) + 1) {
				str = "-0." + std::string(static_cast<size_t>(-Scale) - (str.size() - 1), '0') + str.substr(1);
			}
			else {
				if(!trimzero && ifround) {
					str = str.substr(0, str.size() + Scale) + "." + str.substr(str.size() + Scale, roundprec);
				}
				else {
					str = str.substr(0, str.size() + Scale) + "." + str.substr(str.size() + Scale);
				}
			}
			if(trimzero) {
				rtrim(str, "0");
				if(str[str.size() - 1] == '.') {
					str.erase(str.size() - 1, 1);
				}
			}
			return str;
		}
		else {
			if(Data != 0) {
				return str + std::string(Scale, '0');
			}
			else {
				return str;
			}
		}
	}
	else {
		if(0 == Data) {
			return std::string("0.E0");
		}
		if(Data > 0) {
			return str.substr(0, 1) + "." + rtrim_copy(str.substr(1), "0") + "E" + std::to_string(static_cast<int64_t>(str.size()) - 1 + Scale);
		}
		else {
			return str.substr(0, 2) + "." + rtrim_copy(str.substr(2), "0") + "E" + std::to_string(static_cast<int64_t>(str.size()) - 2 + Scale);
		}
	}
}

void CDecimal64::Set(const float& num)
{
	if(0 == num) {
		Data = 0;
		Scale = 0;
		return;
	}
	if(NotScale == Scale) {
		Set(num_to_string(num));
		return;
	}
	if(0 == Scale) {
		Data = roundf(num);
	}
	else if(Scale < 0) {
		__float128 numd = num;
		for(ScaleType i = 0; i < -Scale; i++) {
			numd *= 10.0Q;
		}
		if(isinfq(numd)) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString2(static_cast<__float128>(num)));
		}
		numd = roundq(numd);
		if(numd > MaxValueInt || numd < MinValueInt) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(static_cast<__float128>(num)));
		}
		Data = numd;
	}
	else {
		float numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0f;
		}
		Data = roundf(numd);
	}
}

void CDecimal64::Set(const double& num)
{
	if(0 == num) {
		Data = 0;
		Scale = 0;
		return;
	}
	if(NotScale == Scale) {
		Set(num_to_string(num));
		return;
	}
	if(0 == Scale) {
		Data = ::round(num);
	}
	else if(Scale < 0) {
		__float128 numd = num;
		for(ScaleType i = 0; i < -Scale; i++) {
			numd *= 10.0Q;
		}
		if(isinfq(numd)) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString2(static_cast<__float128>(num)));
		}
		numd = roundq(numd);
		if(numd > MaxValueInt || numd < MinValueInt) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(static_cast<__float128>(num)));
		}
		Data = numd;
	}
	else {
		double numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0;
		}
		Data = round(numd);
	}
}

void CDecimal64::Set(const __float128& num)
{
	if(0 == num) {
		Data = 0;
		Scale = 0;
		return;
	}
	if(NotScale == Scale) {
		Set(num_to_string(num));
		return;
	}
	if(0 == Scale) {
		Data = roundq(num);
	}
	else if(Scale < 0) {
		__float128 numd = num;
		for(ScaleType i = 0; i < -Scale; i++) {
			numd *= 10.0Q;
		}
		if(isinfq(numd)) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString2(static_cast<__float128>(num)));
		}
		numd = roundq(numd);
		if(numd > MaxValueInt || numd < MinValueInt) {
			ThrowError(ERR_OUT_OF_RANGE, LimitString(static_cast<__float128>(num)));
		}
		Data = numd;
	}
	else {
		__float128 numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0Q;
		}
		Data = roundq(numd);
	}
}

int64_t CDecimal64::ChangeScale(ScaleType newscale) const
{
	int64_t newdata = Data;
	if(Scale > newscale) {
		ScaleType multimes = ToMaxDigits(newdata);
		if(Scale - newscale > multimes) {
			ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when its fractional digits is converted from " + std::to_string(Scale) +" to " + std::to_string(newscale));
		}
		newdata *= pow_10(Scale - newscale);
	}
	else if(Scale < newscale) {
		if(newscale - Scale > MaxPrecision) {
			newdata = 0;
		}
		else {
			ScaleType multimes = newscale - Scale - 1;
			if(multimes > 0) {
				newdata /= pow_10(multimes);
			}
			int8_t lastdigit = newdata % 10;
			newdata /= 10;
			if(lastdigit >= 5) {
				newdata++;
			}
			else if(lastdigit <= -5) {
				newdata--;
			}
		}
	}
	return newdata;
}

int64_t CDecimal64::ToInt64(RoundType rt, bool throwexception)
{
	if(0 == Data) {
		return 0;
	}
	int64_t result = Data;
	if(Scale >= 0) {
		ScaleType multimes = ToMaxDigits(result);
		if(Scale > multimes) {
			if(throwexception) {
				ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int64 (" + num_to_string(MinValueInt) +" to " + num_to_string(MaxValueInt) +") and scale is " + num_to_string(Scale));
			}
			else {
				// 返回最小值表示无效
				return num_limits<int64_t>::min();
			}
		}
		result *= pow_10(Scale);
	}
	else {
		if(-Scale > MaxPrecision) {
			result = 0;
		}
		else {
			if(RT_FLOOR != rt) {
				result /= pow_10(-Scale - 1);
				int8_t lastdigit = result % 10;
				result /= 10;
				if(RT_ROUND == rt) {
					if(lastdigit >= 5) {
						result++;
					}
					else if(lastdigit <= -5) {
						result--;
					}
				}
				// RT_CEIL
				else {
					if(lastdigit > 0) {
						result++;
					}
					else if(lastdigit < 0) {
						result--;
					}
				}
			}
			// RT_FLOOR
			else {
				if(-Scale < MaxPrecision) {
					result /= pow_10(-Scale);
				}
				else {
					result = 0;
				}
			}
		}
	}
	return result;
}

uint64_t CDecimal64::ToUInt64(RoundType rt)
{
	int64_t result = ToInt64(rt);
	if(result < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt64");
	}
	return result;
}

int32_t CDecimal64::ToInt32(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	int64_t result = ToInt64(rt, false);
	if(result > num_limits<int32_t>::max() || result < num_limits<int32_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int32 (" + num_to_string(num_limits<int32_t>::min()) +" to " + num_to_string(num_limits<int32_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint32_t CDecimal64::ToUInt32(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt32");
	}
	int64_t result = ToInt64(rt, false);
	if(result < 0 || result > num_limits<uint32_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt32 (0 to " + num_to_string(num_limits<uint32_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

int16_t CDecimal64::ToInt16(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	int64_t result = ToInt64(rt, false);
	if(result > num_limits<int16_t>::max() || result < num_limits<int16_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int16 (" + num_to_string(num_limits<int16_t>::min()) +" to " + num_to_string(num_limits<int16_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint16_t CDecimal64::ToUInt16(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt16");
	}
	int64_t result = ToInt64(rt, false);
	if(result < 0 || result > num_limits<uint16_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt16 (0 to " + num_to_string(num_limits<uint16_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

int8_t CDecimal64::ToInt8(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	int64_t result = ToInt64(rt, false);
	if(result > num_limits<int8_t>::max() || result < num_limits<int8_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int8 (" + num_to_string(num_limits<int8_t>::min()) +" to " + num_to_string(num_limits<int8_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint8_t CDecimal64::ToUInt8(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt8");
	}
	int64_t result = ToInt64(rt, false);
	if(result < 0 || result > num_limits<uint8_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt8 (0 to " + num_to_string(num_limits<uint8_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

float CDecimal64::ToFloat32()
{
	if(0 == Data) {
		return 0.0f;
	}
	float result = Data;
	if(Scale > 0) {
		for(ScaleType i = 0; i < Scale; i++) {
			result *= 10.0f;
			if(std::isinf(result)) {
				ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Float (" + num_to_string(std::numeric_limits<float>::lowest()) +" to " + num_to_string(std::numeric_limits<float>::max()) +")");
			}
		}
	}
	else if(Scale < 0) {
		if(Scale < -MaxPrecision + std::numeric_limits<float>::min_exponent10) {
			result = 0.0f;
		}
		else {
			for(ScaleType i = 0; i < -Scale; i++) {
				result /= 10.0f;
			}
		}
	}
	return result;
}

double CDecimal64::ToFloat64()
{
	if(0 == Data) {
		return 0.0;
	}
	double result = Data;
	if(Scale > 0) {
		for(ScaleType i = 0; i < Scale; i++) {
			result *= 10.0;
			if(std::isinf(result)) {
				ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Double (" + num_to_string(std::numeric_limits<double>::lowest()) +" to " + num_to_string(std::numeric_limits<double>::max()) +")");
			}
		}
	}
	else if(Scale < 0) {
		if(Scale < -MaxPrecision + std::numeric_limits<double>::min_exponent10) {
			result = 0.0;
		}
		else {
			for(ScaleType i = 0; i < -Scale; i++) {
				result /= 10.0;
			}
		}
	}
	return result;
}

__float128 CDecimal64::ToFloat128()
{
	if(0 == Data) {
		return 0.0Q;
	}
	__float128 result = Data;
	if(Scale > 0) {
		for(ScaleType i = 0; i < Scale; i++) {
			result *= 10.0Q;
			if(isinfq(result)) {
				ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Float128 (" + num_to_string(-FLT128_MAX) +" to " + num_to_string(FLT128_MAX) +")");
			}
		}
	}
	else if(Scale < 0) {
		if(Scale < -MaxPrecision + FLT128_MIN_10_EXP) {
			result = 0.0Q;
		}
		else {
			for(ScaleType i = 0; i < -Scale; i++) {
				result /= 10.0Q;
			}
		}
	}
	return result;
}

bool CDecimal64::Plus(const int64_t& a_data, const int64_t& b_data, int64_t& result, ScaleType& scale)
{
	__int128_t r = __int128_t(a_data) + __int128_t(b_data);
	if(r > MaxValueInt || r < MinValueInt) {
		if(scale == MaxScale) {
			return false;
		}
		else {
			scale++;
			result = r / 10;
			int8_t lastdigit = r % 10;
			if(lastdigit >= 5) {
				result++;
			}
			else if(lastdigit <= -5) {
				result--;
			}
		}
	}
	else {
		result = r;
	}
	return true;
}

CDecimal64 operator+(const CDecimal64 &A, const CDecimal64 &B)
{
	// 由于代码较下面的代码效率稍低，暂不使用，但由于加数之间的大小关系判断很精确所以还留着
	/*if(0 == A.Data) {
		return B;
	}
	if(0 == B.Data) {
		return A;
	}
	if(CDecimal64::NotScale == A.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, A.ToString() + " isn't initialzed");
	}
	if(CDecimal64::NotScale == B.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, B.ToString() + " isn't initialzed");
	}
	CDecimal64 C;
	if(A.Scale == B.Scale) {
		C.Scale = A.Scale;
		if(!CDecimal64::Plus(A.Data, B.Data, C.Data, C.Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	else {
		CDecimal64::ScaleType a_scale;
		CDecimal64::ScaleType b_scale;
		int64_t a_data;
		int64_t b_data;
		if(A.Scale > B.Scale) {
			a_scale = A.Scale;
			b_scale = B.Scale;
			a_data = A.Data;
			b_data = B.Data;
		}
		else {
			a_scale = B.Scale;
			b_scale = A.Scale;
			a_data = B.Data;
			b_data = A.Data;
		}
		CDecimal64::DigitsType a_digits = CDecimal64::GetDigits(a_data);
		CDecimal64::DigitsType b_digits = CDecimal64::GetDigits(b_data);
		if(a_scale - (CDecimal64::MaxPrecision - a_digits) > b_scale + b_digits) {
			C.Data = a_data;
			C.Scale = a_scale;
		}
		else {
			CDecimal64::ScaleType scalediff = a_scale - b_scale;
			CDecimal64::ScaleType i = std::min(scalediff, CDecimal64::ToMaxDigits(a_data));
			if(i > 0) {
				a_data *= CDecimal64::pow_10(i);
				a_scale -= i;
			}
			if(i < scalediff) {
				CDecimal64::ScaleType multimes = scalediff - 1 - i;
				b_scale += multimes + 1;
				if(multimes > 0) {
					b_data /= CDecimal64::pow_10(multimes);
				}
				int8_t lastdigit = b_data % 10;
				b_data /= 10;
				if(lastdigit >= 5) {
					b_data++;
				}
				else if(lastdigit <= -5) {
					b_data--;
				}
			}
			C.Scale = a_scale;
			if(!CDecimal64::Plus(a_data, b_data, C.Data, C.Scale)) {
				ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString() + " and " + B.ToString() + " is out of range");
			}
		}
	}
	return C;*/

	if(0 == A.Data) {
		return B;
	}
	if(0 == B.Data) {
		return A;
	}
	if(CDecimal64::NotScale == A.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, A.ToString() + " isn't initialzed");
	}
	if(CDecimal64::NotScale == B.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, B.ToString() + " isn't initialzed");
	}
	CDecimal64 C;
	if(A.Scale == B.Scale) {
		C.Scale = A.Scale;
		if(!CDecimal64::Plus(A.Data, B.Data, C.Data, C.Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
		}
	}
	// 因为Data最高位的数字为1，所以A与B的Scale相等时四舍五入约等于0，至于大于的情况B也是约为0
	// 对于非__int128_t的整数，如果最大或最小整数的第一个数字如果大于等于5，那么返回值应当加1或减1
	else if(A.Scale - B.Scale >= CDecimal64::MaxPrecision) {
		return A;
	}
	else if(B.Scale - A.Scale >= CDecimal64::MaxPrecision) {
		return B;
	}
	else {
		CDecimal64::ScaleType a_scale;
		CDecimal64::ScaleType b_scale;
		int64_t a_data;
		int64_t b_data;
		if(A.Scale > B.Scale) {
			a_scale = A.Scale;
			b_scale = B.Scale;
			a_data = A.Data;
			b_data = B.Data;
		}
		else {
			a_scale = B.Scale;
			b_scale = A.Scale;
			a_data = B.Data;
			b_data = A.Data;
		}
		CDecimal64::ScaleType scalediff = a_scale - b_scale;
		CDecimal64::ScaleType i = std::min(scalediff, CDecimal64::ToMaxDigits(a_data));
		if(i > 0) {
			a_data *= CDecimal64::pow_10(i);
		}
		a_scale -= i;
		if(i < scalediff) {
			CDecimal64::ScaleType multimes = scalediff - 1 - i;
			b_scale += multimes + 1;
			if(multimes > 0) {
				b_data /= CDecimal64::pow_10(multimes);
			}
			int8_t lastdigit = b_data % 10;
			b_data /= 10;
			if(lastdigit >= 5) {
				b_data++;
			}
			else if(lastdigit <= -5) {
				b_data--;
			}
		}
		C.Scale = a_scale;
		if(!CDecimal64::Plus(a_data, b_data, C.Data, C.Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	return C;
}

void CDecimal64::Shrink(__int128_t& result, ScaleType& scale)
{
	CDecimal64::DigitsType digits = CDecimal64::GetDigits(ceilq(fabsq(__float128(result)) / __float128(CDecimal64::MaxValueInt)));
	if(digits > 1) {
		result /= CDecimal64::pow_10(digits - 1);
	}
	scale += digits;
	int8_t lastdigit = result % 10;
	result /= 10;
	if(lastdigit >= 5) {
		result++;
	}
	else if(lastdigit <= -5) {
		result--;
	}
}

CDecimal64 operator*(const CDecimal64 &A, const CDecimal64 &B)
{
	CDecimal64 C(0, 0);
	CDecimal64::ScaleType scale = A.Scale + B.Scale;
	// 如果数据为0或者Scale之和过小导致乘积太小接近于0则返回0值
	if(0 == A.Data || 0 == B.Data || scale + CDecimal64::MaxPrecision + CDecimal64::MaxPrecision + 1 < CDecimal64::MinScale) {
		return C;
	}
	// 如果数据的Scale过大导致数值出错
	if(scale > CDecimal64::MaxScale) {
		ThrowError(ERR_OUT_OF_RANGE, "Multiplication of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
	}

	C.Scale = scale;
	__int128_t result = __int128_t(A.Data) * __int128_t(B.Data);
	if(result > CDecimal64::MaxValueInt || result < CDecimal64::MinValueInt) {
		CDecimal64::Shrink(result, C.Scale);
	}
	C.Data = result;

	C.MakeScaleEffective();

	return C;
}

CDecimal64 operator/(const CDecimal64 &A, const CDecimal64 &B)
{
	if(0 == B.Data) {
		ThrowError(ERR_DENOMINATOR_ZERO, "Division of " + A.ToString() + " and " + B.ToString() + " can't be computed because the denominator is zero");
	}
	CDecimal64 C(0, 0);
	CDecimal64::ScaleType scale = A.Scale - B.Scale;
	if(0 == A.Data || scale + CDecimal64::MaxPrecision < CDecimal64::MinScale) {
		return C;
	}
	if(scale - CDecimal64::MaxPrecision > CDecimal64::MaxScale + CDecimal64::MaxPrecision - 1) {
		ThrowError(ERR_OUT_OF_RANGE, "Division of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
	}

	C.Scale = scale;
	CDecimal64::DigitsType a_digits = CDecimal64::GetDigits(A.Data);
	CDecimal64::DigitsType b_digits = CDecimal64::GetDigits(B.Data);
	CDecimal64::DigitsType multimes = b_digits + CDecimal64::MaxPrecision - a_digits;
	C.Scale -= multimes;
	__int128_t a_data = A.Data * CDecimal64::pow_10_128(multimes);
	__int128_t result = a_data / B.Data;
	// 如果结果在64位整数范围内
	if(result <= CDecimal64::MaxValueInt && result >= CDecimal64::MinValueInt) {
		__int128_t remainder = a_data % B.Data;
		if(remainder != 0) {
			bool ispositive = ((A.Data >= 0 && B.Data >= 0) || (A.Data < 0 && B.Data < 0));
			remainder = remainder > 0 ? remainder : -remainder;
			// 余数乘以2后如果比除数大就进一
			remainder *= 2;
			if(remainder >= abs(B.Data)) {
				if(ispositive) {
					result++;
				}
				else {
					result--;
				}
			}
		}
	}
	// 如果结果值较大，最多可能会运行3次
	while(result > CDecimal64::MaxValueInt || result < CDecimal64::MinValueInt) {
		C.Scale++;
		int8_t lastdigit = result % 10;
		result /= 10;
		if(lastdigit >= 5) {
			result++;
		}
		else if(lastdigit <= -5) {
			result--;
		}
	}

	C.Data = result;

	C.MakeScaleEffective();
	return C;
}

void CDecimal64::MakeScaleEffective()
{
	if(Scale < MinScale) {
		ScaleType multimes = MinScale - Scale;
		Scale = MinScale;
		if(multimes > 19) {
			Data = 0;
			return;
		}
		if(multimes > 1) {
			Data /= CDecimal64::pow_10(multimes - 1);
		}
		int8_t lastdigit = Data % 10;
		Data /= 10;
		if(lastdigit >= 5) {
			Data++;
		}
		else if(lastdigit <= -5) {
			Data--;
		}
	}
	else if(Scale > MaxScale) {
		ScaleType multimes = Scale - MaxScale;
		DigitsType diffdig = CDecimal64::ToMaxDigits(Data);
		if(diffdig < multimes) {
			ThrowError(ERR_OUT_OF_RANGE, "Number '" + std::to_string(Data) + "E" + std::to_string(Scale) + "' is out of range (" + DataRangeString + ")");
		}
		else {
			Data *= pow_10(multimes);
			Scale = MaxScale;
		}
	}
}

}
