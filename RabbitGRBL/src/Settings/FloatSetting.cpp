#include "../Grbl.h"

FloatSetting::FloatSetting(const char *description,
                           ERabbitGRBLItemType type,
                           const char *grblName,
                           const char *name,
                           float defVal,
                           float minVal,
                           float maxVal,
                           bool (*checker)(char *) = NULL) : Setting(description, type, grblName, name, checker),
                                                             _defaultValue(defVal), _currentValue(defVal), _minValue(minVal), _maxValue(maxVal) {}

void FloatSetting::load()
{
    // Read the stored float value from NVS
    if (!NVSManager::ReadFloat(_keyName, &_currentValue))
    {
        _currentValue = _defaultValue;
    }
}

void FloatSetting::setDefault()
{
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue)
    {
        NVSManager::EraseKey(_keyName);
    }
}

EError FloatSetting::setStringValue(char *s)
{
    s = trim(s);
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }

    float convertedValue;
    uint8_t len = strlen(s);
    uint8_t retlen = 0;
    if (!read_float(s, &retlen, &convertedValue) || retlen != len)
    {
        return EError::BadNumberFormat;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue)
    {
        return EError::NumberRange;
    }
    _currentValue = convertedValue;
    if (_storedValue != _currentValue)
    {
        if (_currentValue == _defaultValue)
        {
            NVSManager::EraseKey(_keyName);
        }
        else
        {
            if (NVSManager::WriteFloat(_keyName, _currentValue) != EError::Ok)
            {
                return EError::NvsSetFailed;
            }
            _storedValue = _currentValue;
        }
    }
    check(NULL);
    return EError::Ok;
}

const char *FloatSetting::getDefaultString()
{
    static char strval[32];
    (void)sprintf(strval, "%.3f", _defaultValue);
    return strval;
}

const char *FloatSetting::getStringValue()
{
    static char strval[32];
    (void)sprintf(strval, "%.3f", get());
#if 0
    // With the goal of representing both large and small floating point
    // numbers compactly while showing clearly that the are floating point,
    // remove trailing zeros leaving at least one post-decimal digit.
    // The loop is guaranteed to terminate because the string contains
    // a decimal point which is not a '0'.
    for (char *p = strval + strlen(strval) - 1; *p == '0'; --p) {
        if (*(p-1) != '.' && *(p-1) != ',') {
            *p = '\0';
        }
    }
#endif
    return strval;
}
