#include "cdecimal128.h"

namespace MoonDb {

const CDecimal128::ScaleType CDecimal128::NotScale = -32768;
const CDecimal128::ScaleType CDecimal128::MinScale = -32767;
const CDecimal128::ScaleType CDecimal128::MaxScale = 32729;
const CDecimal128::DigitsType CDecimal128::MaxScaleLength = 18;
const CDecimal128::DigitsType CDecimal128::MaxPrecision = 39;
const CDecimal128::DigitsType CDecimal128::MaxPrecisionU = 39;
const __int128_t CDecimal128::MaxValueInt = (__int128_t(9223372036854775807LL) << 64) + __int128_t(18446744073709551615ULL);	/**< 128位整数最大值 */
const __int128_t CDecimal128::MinValueInt = - (__int128_t(9223372036854775807LL) << 64) - __int128_t(18446744073709551615ULL);
const std::string CDecimal128::MaxValueString = "170141183460469231731687303715884105727";
const std::string CDecimal128::MinValueString = "-170141183460469231731687303715884105727";
const std::string CDecimal128::DataRangeString = "from -1.70141183460469231731687303715884105727E32767 to 1.70141183460469231731687303715884105727E32767";
const __int128_t CDecimal128::MaxDivInt = __int128_t(9999999999999999999ULL) * __int128_t(10000000000000000000ULL) + __int128_t(9999999999999999999ULL);
const __int128_t CDecimal128::MinDivInt = -__int128_t(9999999999999999999ULL) * __int128_t(10000000000000000000ULL) - __int128_t(9999999999999999999ULL);

void CDecimal128::Set(const std::string& num)
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

void CDecimal128::Set(const __int128_t& num)
{
	if(NotScale == Scale) {
		Scale = 0;
	}
	if(0 == num) {
		Data = 0;
	}
	else if(0 == Scale) {
		Data = num;
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
	//std::cout << Data << std::endl;
}

void CDecimal128::Set(const __uint128_t& num)
{
	if(NotScale == Scale) {
		Scale = 0;
	}
	Data = num;
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
				__uint128_t n = num;
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
	//std::cout << Data << std::endl;
}

std::string CDecimal128::ToString(bool iffixed, bool trimzero, ScaleType roundprec) const noexcept
{
	__int128_t data = Data;
	bool ifround = roundprec != NotScale && roundprec < -Scale;
	if(ifround) {
		if(-Scale - roundprec > MaxPrecision) {
			data = 0;
		}
		else {
			__int128_t mul = pow_10(-Scale - 1 - roundprec);
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

void CDecimal128::Set(const float& num)
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
			ThrowError(ERR_OUT_OF_RANGE, LimitString(static_cast<__float128>(num)));
		}
		Data = roundq(numd);
	}
	else {
		float numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0f;
		}
		Data = roundf(numd);
	}
}

void CDecimal128::Set(const double& num)
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
			ThrowError(ERR_OUT_OF_RANGE, LimitString(static_cast<__float128>(num)));
		}
		Data = roundq(numd);
	}
	else {
		double numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0;
		}
		Data = round(numd);
	}
}

void CDecimal128::Set(const __float128& num)
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
			ThrowError(ERR_OUT_OF_RANGE, LimitString(num));
		}
		Data = roundq(numd);
	}
	else {
		__float128 numd = num;
		for(ScaleType i = 0; i < Scale; i++) {
			numd /= 10.0Q;
		}
		Data = roundq(numd);
	}
}

__int128_t CDecimal128::ChangeScale(ScaleType newscale) const
{
	__int128_t newdata = Data;
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

__int128_t CDecimal128::ToInt128(RoundType rt, bool throwexception)
{
	if(0 == Data) {
		return 0;
	}
	__int128_t result = Data;
	if(Scale >= 0) {
		ScaleType multimes = ToMaxDigits(result);
		if(Scale > multimes) {
			if(throwexception) {
				ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int128 (" + num_to_string(MinValueInt) +" to " + num_to_string(MaxValueInt) +") and scale is " + num_to_string(Scale));
			}
			else {
				return num_limits<__int128_t>::min();
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

__uint128_t CDecimal128::ToUInt128(RoundType rt)
{
	__int128_t result = ToInt128(rt, false);
	if(result < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt128");
	}
	return result;
}

int64_t CDecimal128::ToInt64(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	__int128_t result = ToInt128(rt, false);
	if(result > num_limits<int64_t>::max() || result < num_limits<int64_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int64 (" + num_to_string(num_limits<int64_t>::min()) +" to " + num_to_string(num_limits<int64_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint64_t CDecimal128::ToUInt64(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt64");
	}
	__int128_t result = ToInt128(rt, false);
	if(result < 0 || result > num_limits<uint64_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt64 (0 to " + num_to_string(num_limits<uint64_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

int32_t CDecimal128::ToInt32(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	__int128_t result = ToInt128(rt, false);
	if(result > num_limits<int32_t>::max() || result < num_limits<int32_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int32 (" + num_to_string(num_limits<int32_t>::min()) +" to " + num_to_string(num_limits<int32_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint32_t CDecimal128::ToUInt32(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt32");
	}
	__int128_t result = ToInt128(rt, false);
	if(result < 0 || result > num_limits<uint32_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt32 (0 to " + num_to_string(num_limits<uint32_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

int16_t CDecimal128::ToInt16(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	__int128_t result = ToInt128(rt, false);
	if(result > num_limits<int16_t>::max() || result < num_limits<int16_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int16 (" + num_to_string(num_limits<int16_t>::min()) +" to " + num_to_string(num_limits<int16_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint16_t CDecimal128::ToUInt16(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt16");
	}
	__int128_t result = ToInt128(rt, false);
	if(result < 0 || result > num_limits<uint16_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt16 (0 to " + num_to_string(num_limits<uint16_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

int8_t CDecimal128::ToInt8(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	__int128_t result = ToInt128(rt, false);
	if(result > num_limits<int8_t>::max() || result < num_limits<int8_t>::min()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to Int16 (" + num_to_string(num_limits<int8_t>::min()) +" to " + num_to_string(num_limits<int8_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

uint8_t CDecimal128::ToUInt8(RoundType rt)
{
	if(0 == Data) {
		return 0;
	}
	if(Data < 0) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is less than 0 when it is converted to UInt8");
	}
	__int128_t result = ToInt128(rt, false);
	if(result < 0 || result > num_limits<uint8_t>::max()) {
		ThrowError(ERR_OUT_OF_RANGE, "Number '" + ToString() + "' is out of range when it is converted to UInt8 (0 to " + num_to_string(num_limits<uint8_t>::max()) + ") and scale is " + num_to_string(Scale));
	}
	return result;
}

float CDecimal128::ToFloat32()
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

double CDecimal128::ToFloat64()
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

__float128 CDecimal128::ToFloat128()
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

bool CDecimal128::Plus(const __int128_t& a_data, const __int128_t& b_data, __int128_t& result, ScaleType& scale)
{
	result = a_data + b_data;
	// 如果result数值错误，则将A.Data和B.Data都除以10并且将相应的Scale加1之后再相加，并判断除以10的余数修正result数值
	if((a_data > 0 && b_data > 0 && result <= 0) ||
		(a_data < 0 && b_data < 0 && result >= 0)) {
		if(scale == MaxScale) {
			return false;
		}
		else {
			scale++;
			result = a_data / 10 + b_data / 10;
			int8_t remainder = a_data % 10 + b_data % 10;
			if(remainder >= 15) {
				result += 2;
			}
			else if(remainder >= 5) {
				result++;
			}
			else if(remainder <= -15) {
				result -= 2;
			}
			else if(remainder <= -5) {
				result--;
			}
		}
	}
	return true;
}

CDecimal128 & CDecimal128::operator+=(const CDecimal128 &B)
{
	if(0 == B.Data) {
		return *this;
	}
	if(CDecimal128::NotScale == B.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, B.ToString() + " isn't initialzed");
	}
	if(Scale == B.Scale) {
		if(!CDecimal128::Plus(Data, B.Data, Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	// 因为Data最高位的数字为1，所以A与B的Scale相等时四舍五入约等于0，至于大于的情况B也是约为0
	// 对于非__int128_t的整数，如果最大或最小整数的第一个数字如果大于等于5，那么返回值应当加1或减1
	else if(Scale - B.Scale >= CDecimal128::MaxPrecision) {
		return *this;
	}
	else if(B.Scale - Scale >= CDecimal128::MaxPrecision) {
		*this = B;
		return *this;
	}
	else {
		CDecimal128::ScaleType a_scale;
		CDecimal128::ScaleType b_scale;
		__int128_t a_data;
		__int128_t b_data;
		if(Scale > B.Scale) {
			a_scale = Scale;
			b_scale = B.Scale;
			a_data = Data;
			b_data = B.Data;
		}
		else {
			a_scale = B.Scale;
			b_scale = Scale;
			a_data = B.Data;
			b_data = Data;
		}
		CDecimal128::ScaleType scalediff = a_scale - b_scale;
		CDecimal128::ScaleType i = std::min(scalediff, CDecimal128::ToMaxDigits(a_data));
		if(i > 0) {
			a_data *= CDecimal128::pow_10(i);
		}
		a_scale -= i;
		if(i < scalediff) {
			CDecimal128::ScaleType multimes = scalediff - 1 - i;
			b_scale += multimes + 1;
			if(multimes > 0) {
				b_data /= CDecimal128::pow_10(multimes);
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
		Scale = a_scale;
		if(!CDecimal128::Plus(a_data, b_data, Data, Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	return *this;
}

CDecimal128 operator+(const CDecimal128 &A, const CDecimal128 &B)
{
	if(0 == A.Data) {
		return B;
	}
	if(0 == B.Data) {
		return A;
	}
	if(CDecimal128::NotScale == A.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, A.ToString() + " isn't initialzed");
	}
	if(CDecimal128::NotScale == B.Scale) {
		ThrowError(ERR_NOT_INITIALIZE, B.ToString() + " isn't initialzed");
	}
	CDecimal128 C;
	if(A.Scale == B.Scale) {
		C.Scale = A.Scale;
		if(!CDecimal128::Plus(A.Data, B.Data, C.Data, C.Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	// 因为Data最高位的数字为1，所以A与B的Scale相等时四舍五入约等于0，至于大于的情况B也是约为0
	// 对于非__int128_t的整数，如果最大或最小整数的第一个数字如果大于等于5，那么返回值应当加1或减1
	else if(A.Scale - B.Scale >= CDecimal128::MaxPrecision) {
		return A;
	}
	else if(B.Scale - A.Scale >= CDecimal128::MaxPrecision) {
		return B;
	}
	else {
		CDecimal128::ScaleType a_scale;
		CDecimal128::ScaleType b_scale;
		__int128_t a_data;
		__int128_t b_data;
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
		CDecimal128::ScaleType scalediff = a_scale - b_scale;
		CDecimal128::ScaleType i = std::min(scalediff, CDecimal128::ToMaxDigits(a_data));
		if(i > 0) {
			a_data *= CDecimal128::pow_10(i);
		}
		a_scale -= i;
		if(i < scalediff) {
			CDecimal128::ScaleType multimes = scalediff - 1 - i;
			b_scale += multimes + 1;
			if(multimes > 0) {
				b_data /= CDecimal128::pow_10(multimes);
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
		if(!CDecimal128::Plus(a_data, b_data, C.Data, C.Scale)) {
			ThrowError(ERR_OUT_OF_RANGE, "Addtion of " + A.ToString() + " and " + B.ToString() + " is out of range");
		}
	}
	return C;
}

bool CDecimal128::ChangeScaleForMultiplication(__uint128_t& data, ScaleType oldscale, ScaleType newscale)
{
	if(oldscale > newscale) {
		ScaleType multimes = ToMaxDigits(data);
		if(oldscale - newscale > multimes) {
			return false;
		}
		data *= pow_10(oldscale - newscale);
	}
	else if(oldscale < newscale) {
		if(newscale - oldscale > MaxPrecision) {
			data = 0;
		}
		else {
			ScaleType multimes = newscale - oldscale - 1;
			if(multimes > 0) {
				data /= pow_10(multimes);
			}
			int8_t lastdigit = data % 10;
			data /= 10;
			if(lastdigit >= 5) {
				data++;
			}
			else if(lastdigit <= -5) {
				data--;
			}
		}
	}
	return true;
}

bool CDecimal128::Plus(__int128_t& a_data, const __int128_t& b_data, ScaleType& scale)
{
	__int128_t result = a_data + b_data;
	// 如果result数值错误，则将A.Data和B.Data都除以10并且将相应的Scale加1之后再相加，并判断除以10的余数修正result数值
	if((a_data > 0 && b_data > 0 && result <= 0) ||
		(a_data < 0 && b_data < 0 && result >= 0)) {
		if(scale == MaxScale) {
			return false;
		}
		else {
			scale++;
			result = a_data / 10 + b_data / 10;
			int8_t remainder = a_data % 10 + b_data % 10;
			if(remainder >= 15) {
				result += 2;
			}
			else if(remainder >= 5) {
				result++;
			}
			else if(remainder <= -15) {
				result -= 2;
			}
			else if(remainder <= -5) {
				result--;
			}
		}
	}
	a_data = result;
	return true;
}

void CDecimal128::SetGMPInt(mpz_t mpz, const __uint128_t& data)
{
	const uint32_t* data32 = static_cast<const uint32_t*>(static_cast<const void*>(&data));
	mpz_t t;
	mpz_init_set_ui(mpz, 0);
	mpz_init_set_ui(t, 0);
	uint32_t shift = 0;
	// 加上最低的32位整数
	mpz_set_ui(t, data32[0]);
	mpz_add(mpz, mpz, t);
	// 加上第2低的32位整数
	shift += std::numeric_limits<unsigned long>::digits;
	if(data32[1] != 0) {
		mpz_set_ui(t, data32[1]);
		mpz_mul_2exp(t, t, shift);
		mpz_add(mpz, mpz, t);
	}
	// 加上第3低的32位整数
	shift += std::numeric_limits<unsigned long>::digits;
	if(data32[2] != 0) {
		mpz_set_ui(t, data32[2]);
		mpz_mul_2exp(t, t, shift);
		mpz_add(mpz, mpz, t);
	}
	// 加上最高的32位整数
	if(data32[3] != 0) {
		shift += std::numeric_limits<unsigned long>::digits;
		mpz_set_ui(t, data32[3]);
		mpz_mul_2exp(t, t, shift);
		mpz_add(mpz, mpz, t);
	}
}

CDecimal128 operator*(const CDecimal128 &A, const CDecimal128 &B)
{
	static uint64_t elem_mask = 10000000000000000000ULL;
	static __uint128_t elem_mask_square = __uint128_t(10000000000000000000ULL) * __uint128_t(10000000000000000000ULL);
	CDecimal128 C(0, 0);
	// 如果数据为0或者Scale之和过小导致乘积太小接近于0则返回0值
	if(0 == A.Data || 0 == B.Data || A.Scale + B.Scale + CDecimal128::MaxPrecision + CDecimal128::MaxPrecision + 1 < CDecimal128::MinScale) {
		return C;
	}
	// 如果数据的Scale过大导致数值出错
	if(A.Scale + B.Scale > CDecimal128::MaxScale) {
		ThrowError(ERR_OUT_OF_RANGE, "Multiplication of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
	}

	// 判断是否为负值
	bool isnegative = (A.Data > 0 && B.Data < 0) || (A.Data < 0 && B.Data > 0);
	__uint128_t a_data = A.Data >= 0 ? A.Data : -A.Data;
	__uint128_t b_data = B.Data >= 0 ? B.Data : -B.Data;


	/*********************************************************************************/
	if(a_data <= num_limits<uint64_t>::max() && b_data <= num_limits<uint64_t>::max()) {
		__uint128_t result = a_data * b_data;
		C.Scale = A.Scale + B.Scale;
		if(result > static_cast<__uint128_t>(num_limits<__int128_t>::max())) {
			C.Scale++;
			CDecimal128::ChangeScaleForMultiplication(result, C.Scale, C.Scale + 1);
		}
		C.Data = result;
	}
	else if(a_data < elem_mask_square && b_data < elem_mask_square) {
		uint64_t a_list[2], b_list[2];
		if(a_data < elem_mask) {
			a_list[0] = a_data;
			a_list[1] = 0;
		}
		else if(a_data < elem_mask_square) {
			a_list[0] = a_data % elem_mask;
			a_list[1] = a_data / elem_mask;
		}
		else {
			a_list[0] = a_data % elem_mask;
			a_list[1] = (a_data % elem_mask_square) / elem_mask;
		}
		if(b_data < elem_mask) {
			b_list[0] = b_data;
			b_list[1] = 0;
		}
		else if(b_data < elem_mask_square) {
			b_list[0] = b_data % elem_mask;
			b_list[1] = b_data / elem_mask;
		}
		else {
			b_list[0] = b_data % elem_mask;
			b_list[1] = (b_data % elem_mask_square) / elem_mask;
		}
		// 低字节直接赋值
		C.Scale = A.Scale + B.Scale;
		C.Data = __int128_t(a_list[0]) * __int128_t(b_list[0]);
		CDecimal128 t1, t2;
		t1.Scale = A.Scale + B.Scale + 19;
		t1.Data = __int128_t(a_list[0]) * __int128_t(b_list[1]);
		t2.Scale = A.Scale + B.Scale + 19;
		t2.Data = __int128_t(a_list[1]) * __int128_t(b_list[0]);
		C += t1 + t2;
		t1.Scale = A.Scale + B.Scale + 38;
		t1.Data = __int128_t(a_list[1]) * __int128_t(b_list[1]);
		C += t1;
//		__uint128_t c_list[2], t;
//		t = __uint128_t(a_list[0]) * __uint128_t(b_list[1]) + __uint128_t(a_list[1]) * __uint128_t(b_list[0]);
//		c_list[0] = __uint128_t(a_list[0]) * __uint128_t(b_list[0]) + t % elem_mask * elem_mask;
//		c_list[1] = __uint128_t(a_list[1]) * __uint128_t(b_list[1]) + t / elem_mask + c_list[0] / elem_mask_square;
//		c_list[0] %= elem_mask_square;
//		//std::cout << c_list[1] << c_list[0] << std::endl;
//		CDecimal128::DigitsType digits_high = CDecimal128::GetDigits(c_list[1]);
//		CDecimal128::DigitsType digits_low = 38 - digits_high;
//		C.Scale = A.Scale + B.Scale + digits_high;
//		t = c_list[0] / CDecimal128::pow_10(digits_high - 1);
//		C.Data = c_list[1] * CDecimal128::pow_10(digits_low) + t / 10;
//		int8_t lastdigit = t % 10;
//		if(lastdigit >= 5) {
//			C.Data++;
//		}
	}
	else {
		uint64_t a_list[3], b_list[3];
		if(a_data < elem_mask) {
			a_list[0] = a_data;
			a_list[1] = 0;
			a_list[2] = 0;
		}
		else if(a_data < elem_mask_square) {
			a_list[0] = a_data % elem_mask;
			a_list[1] = a_data / elem_mask;
			a_list[2] = 0;
		}
		else {
			a_list[0] = a_data % elem_mask;
			a_list[1] = (a_data % elem_mask_square) / elem_mask;
			a_list[2] = a_data / elem_mask_square;
		}
		if(b_data < elem_mask) {
			b_list[0] = b_data;
			b_list[1] = 0;
			b_list[2] = 0;
		}
		else if(b_data < elem_mask_square) {
			b_list[0] = b_data % elem_mask;
			b_list[1] = b_data / elem_mask;
			b_list[2] = 0;
		}
		else {
			b_list[0] = b_data % elem_mask;
			b_list[1] = (b_data % elem_mask_square) / elem_mask;
			b_list[2] = b_data / elem_mask_square;
		}
		// 低字节直接赋值
		C.Scale = A.Scale + B.Scale;
		C.Data = __int128_t(a_list[0]) * __int128_t(b_list[0]);
		CDecimal128 t1, t2, t3;
		t1.Scale = A.Scale + B.Scale + 19;
		t1.Data = __int128_t(a_list[0]) * __int128_t(b_list[1]);
		t2.Scale = A.Scale + B.Scale + 19;
		t2.Data = __int128_t(a_list[1]) * __int128_t(b_list[0]);
		C += t1 + t2;
		t1.Scale = A.Scale + B.Scale + 38;
		t1.Data = __int128_t(a_list[1]) * __int128_t(b_list[1]);
		t2.Scale = A.Scale + B.Scale + 38;
		t2.Data = __int128_t(a_list[0]) * __int128_t(b_list[2]);
		t3.Scale = A.Scale + B.Scale + 38;
		t3.Data = __int128_t(a_list[2]) * __int128_t(b_list[0]);
		C += t1 + t2 + t3;
		t1.Scale = A.Scale + B.Scale + 57;
		t1.Data = __int128_t(a_list[1]) * __int128_t(b_list[2]);
		t2.Scale = A.Scale + B.Scale + 57;
		t2.Data = __int128_t(a_list[2]) * __int128_t(b_list[1]);
		C += t1 + t2;
		t1.Scale = A.Scale + B.Scale + 76;
		t1.Data = __int128_t(a_list[2] * b_list[2]);
		C += t1;
	}
	/*********************************************************************************/

	/*********************************************************************************/
	// 用gmp库实现的版本，效率稍低暂不使用，但很通用；boost库的number cpp_dec_float乘法效率很高但数据存储似乎是数组不利于存取和排序，所以牺牲下乘法的效率
	/*mpz_t a_mpz, b_mpz, c_mpz;
	CDecimal128::SetGMPInt(a_mpz, a_data);
	CDecimal128::SetGMPInt(b_mpz, b_data);
	mpz_init_set_ui(c_mpz, 0);
	mpz_mul(c_mpz, a_mpz, b_mpz);
	// 按照乘积的uint64_t的个数分开处理
	switch(c_mpz[0]._mp_size) {
	case 1:
		C.Data = c_mpz[0]._mp_d[0];
		break;
	case 2:
		C.Data = __uint128_t(c_mpz[0]._mp_d[0]) + (__uint128_t(c_mpz[0]._mp_d[1]) << 64);
		if(C.Data <= 0) {
			unsigned long remainder = mpz_tdiv_q_ui(c_mpz, c_mpz, 10);
			C.Data = __int128_t(c_mpz[0]._mp_d[0]) + (__int128_t(c_mpz[0]._mp_d[1]) << 64);
			if(remainder >= 5) {
				C.Data++;
			}
			C.Scale++;
		}
		break;
	default:
		if(3 == c_mpz[0]._mp_size) {
			CDecimal128::ScaleType digits = CDecimal128::GetDigits(c_mpz[0]._mp_d[2]);
			if(digits > 1) {
				//mpz_tdiv_q_ui(c_mpz, c_mpz, CDecimal128::pow_10(digits - 1));
				CDecimal128::gmp_int_div_10(c_mpz, digits - 1);
				C.Scale += digits - 1;
			}
		}
		// 4
		else {
			__uint128_t high = __int128_t(c_mpz[0]._mp_d[2]) + (__int128_t(c_mpz[0]._mp_d[3]) << 64);
			if(high > static_cast<__uint128_t>(num_limits<__int128_t>::max())) {
				if(CDecimal128::MaxPrecisionU > CDecimal128::MaxPrecision) {
					mpz_tdiv_q_ui(c_mpz, c_mpz, 10);
					C.Scale++;
				}
				//mpz_tdiv_q_ui(c_mpz, c_mpz, CDecimal128::pow_10(CDecimal128::MaxPrecision - 1));
				CDecimal128::gmp_int_div_10(c_mpz, CDecimal128::MaxPrecision - 1);
				C.Scale += CDecimal128::MaxPrecision - 1;
			}
			else {
				CDecimal128::ScaleType digits = CDecimal128::GetDigits(static_cast<__int128_t>(high));
				if(digits > 1) {
					//mpz_tdiv_q_ui(c_mpz, c_mpz, CDecimal128::pow_10(digits - 1));
					CDecimal128::gmp_int_div_10(c_mpz, digits - 1);
					C.Scale += digits - 1;
				}
			}
		}
		while(true) {
			unsigned long remainder = mpz_tdiv_q_ui(c_mpz, c_mpz, 10);
			C.Scale++;
			if(2 == c_mpz[0]._mp_size) {
				C.Data = __uint128_t(c_mpz[0]._mp_d[0]) + (__uint128_t(c_mpz[0]._mp_d[1]) << 64);
				if(C.Data >= 0 && remainder >= 5) {
					C.Data++;
				}
				// 此处不加else因为之前可能加1之后会导致负数，所以必须再判断一次
				if(C.Data <= 0) {
					unsigned long remainder = mpz_tdiv_q_ui(c_mpz, c_mpz, 10);
					C.Data = __int128_t(c_mpz[0]._mp_d[0]) + (__int128_t(c_mpz[0]._mp_d[1]) << 64);
					if(remainder >= 5) {
						C.Data++;
					}
					C.Scale++;
				}
				break;
			}
		}
	}

	mpz_clear(a_mpz);
	mpz_clear(b_mpz);
	mpz_clear(c_mpz);*/
	/**********************************************************************************/

	if(isnegative) {
		C.Data = -C.Data;
	}

	//std::cout << C.Scale << "," << CDecimal128::MaxScale << std::endl;
	if(C.Scale > CDecimal128::MaxScale) {
		CDecimal128::ScaleType digits = CDecimal128::GetDigits(C.Data);
		if(CDecimal128::MaxScale + CDecimal128::MaxPrecision - digits >= C.Scale) {
			C.Data = C.ChangeScale(CDecimal128::MaxScale);
			C.Scale = CDecimal128::MaxScale;
		}
		else {
			ThrowError(ERR_OUT_OF_RANGE, "Multiplication of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
		}
	}

	return C;
}

CDecimal128 operator/(const CDecimal128 &A, const CDecimal128 &B)
{
	if(0 == B.Data) {
		ThrowError(ERR_DENOMINATOR_ZERO, "Division of " + A.ToString() + " and " + B.ToString() + " can't be computed because the denominator is zero");
	}
	CDecimal128 C(0, 0);
	CDecimal128::ScaleType scale = A.Scale - B.Scale;
	if(0 == A.Data || scale + CDecimal128::MaxPrecision < CDecimal128::MinScale) {
		return C;
	}
	if(scale - CDecimal128::MaxPrecision > CDecimal128::MaxScale + CDecimal128::MaxPrecision - 1) {
		ThrowError(ERR_OUT_OF_RANGE, "Division of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
	}

	// 使用boost库计算除法
	number<cpp_dec_float<80>> a_num, b_num, c_num;
	a_num = cpp_dec_float<80>(A.ToString(false).c_str());
	b_num = cpp_dec_float<80>(B.ToString(false).c_str());
	c_num = a_num / b_num;
	C.SetScale(CDecimal128::NotScale);
	C.Set(c_num.str(40));

	// 自己编写的除法
	// 判断是否为负值
	/*bool isnegative = (A.Data > 0 && B.Data < 0) || (A.Data < 0 && B.Data > 0);
	__uint128_t a_data = A.Data >= 0 ? A.Data : -A.Data;
	__uint128_t b_data = B.Data >= 0 ? B.Data : -B.Data;
	std::string a_str = num_to_string(a_data);
	std::string b_str = num_to_string(b_data);
	// 考虑到数字开始可能小于除数首位为0，以及末尾数字四舍五入，所以长度加2
	const CDecimal128::DigitsType max_str_len = CDecimal128::MaxPrecision + 2;
	std::string c_str(max_str_len, '\0');
	CDecimal128::DigitsType a_enlarge = b_str.size() + max_str_len - a_str.size();
	pad_right(a_str, b_str.size() + max_str_len, '0');
	size_t b_length = b_str.size();
	std::string b_str0 = "0" + b_str;
	bool flag0 = true;
	for(size_t i = 0; i < max_str_len; i++) {
		c_str[i] = '0';
		if(flag0) {
			if(strncmp(a_str.c_str() + i, b_str.c_str(), b_length) >= 0) {
				c_str[i] = '0';
				do {
					for(size_t j = 0; j < b_length; j++) {
						a_str[i + j] = a_str[i + j] - b_str[j] + '0';
					}
					for(size_t j = b_length - 1; j > 0; j--) {
						if(a_str[i + j] < '0') {
							a_str[i + j] += 10;
							a_str[i + j - 1]--;
						}
					}
					c_str[i]++;
				} while(strncmp(a_str.c_str() + i, b_str.c_str(), b_length) >= 0);
			}
			else {
				flag0 = false;
			}
		}
		else {
			while(strncmp(a_str.c_str() + i - 1, b_str0.c_str(), b_length + 1) >= 0) {
				for(size_t j = 0; j < b_length; j++) {
					a_str[i + j] = a_str[i + j] - b_str[j] + '0';
				}
				for(size_t j = a_str.size() - 1; j > 0; j--) {
					if(a_str[j] < '0') {
						a_str[j] += 10;
						a_str[j - 1]--;
					}
				}
				c_str[i]++;
			};
		}
	}

	// 使用字符串形式进行识别
	//C.SetScale(CDecimal128::NotScale);
	//C.Set((isnegative ? "-" : "") + c_str + "E" + num_to_string(scale - a_enlarge + 1));

	// 适用单独的代码进行识别计算结果
	std::string numint = c_str.c_str();
	if('0' == numint[0]) {
		numint.erase(0, 1);
	}
	if(isnegative) {
		numint = "-" + numint;
	}
	int64_t numintsize = static_cast<int64_t>(numint.size());
	char lastdigit = '0';
	int64_t se = scale - a_enlarge + 1;
	// 负数
	if(isnegative) {
		if(numintsize > CDecimal128::MaxPrecision + 1) {
			numintsize = CDecimal128::MaxPrecision + 1;
			se += numint.size() - numintsize;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
		if(CDecimal128::MaxPrecision + 1 == numintsize && CDecimal128::CheckIfIntStringOutOfRange(numint)) {
			numintsize = CDecimal128::MaxPrecision;
			se += numint.size() - numintsize;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
	}
	//正数
	else {
		if(numintsize > CDecimal128::MaxPrecision) {
			numintsize = CDecimal128::MaxPrecision;
			se += numint.size() - numintsize;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
		if(CDecimal128::MaxPrecision == numintsize && CDecimal128::CheckIfIntStringOutOfRange(numint)) {
			numintsize = CDecimal128::MaxPrecision - 1;
			se += numint.size() - numintsize;
			lastdigit = numint[numintsize];
			numint = numint.substr(0, numintsize);
		}
	}
	int64_t scaleexp = se + numintsize - (isnegative ? 1 : 0);
	if(scaleexp > CDecimal128::MaxScale + CDecimal128::MaxPrecision) {
		ThrowError(ERR_OUT_OF_RANGE, "Division of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
	}
	else if(scaleexp < CDecimal128::MinScale) {
		C.Scale = 0;
		C.Data = 0;
	}
	else if(CDecimal128::MinScale == scaleexp) {
		if(numint[(isnegative ? 1 : 0)] >= '5') {
			C.Scale = scaleexp;
			if(isnegative) {
				C.Data = -1;
			}
			else {
				C.Data = 1;
			}
		}
		else {
			C.Scale = 0;
			C.Data = 0;
		}
	}
	else {
		C.Scale = se;
		C.Data = stolll(numint);
		// 如果已达到最大或最小值就不再进一，而是将其舍掉，否则如果提高Scale的值会导致精度的进一步丢失
		if(lastdigit >= '5') {
			if(isnegative && C.Data > CDecimal128::MinValueInt) {
				C.Data--;
			}
			else if(!isnegative && C.Data < CDecimal128::MaxValueInt) {
				C.Data++;
			}
		}
	}*/

	//std::cout << C.Scale << "," << CDecimal128::MaxScale << std::endl;
	if(C.Scale > CDecimal128::MaxScale) {
		CDecimal128::ScaleType digits = CDecimal128::GetDigits(C.Data);
		if(CDecimal128::MaxScale + CDecimal128::MaxPrecision - digits >= C.Scale) {
			C.Data = C.ChangeScale(CDecimal128::MaxScale);
			C.Scale = CDecimal128::MaxScale;
		}
		else {
			ThrowError(ERR_OUT_OF_RANGE, "Division of " + A.ToString(false) + " and " + B.ToString(false) + " is out of range");
		}
	}

	return C;
}

}
