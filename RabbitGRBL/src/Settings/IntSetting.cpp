#include "../Grbl.h"

IntSetting::IntSetting(const char *description,
                       ERabbitGRBLItemType type,
                       const char *grblName,
                       const char *name,
                       int32_t defVal,
                       int32_t minVal,
                       int32_t maxVal,
                       bool (*checker)(char *) = NULL,
                       bool currentIsNvm) : Setting(description, type, grblName, name, checker),
                                            _defaultValue(defVal), _currentValue(defVal), _minValue(minVal), _maxValue(maxVal), _currentIsNvm(currentIsNvm)
{
    _storedValue = std::numeric_limits<int32_t>::min();
}

void IntSetting::load()
{
    // Read the stored int32 value from NVS
    if (!NVSManager::ReadInt(_keyName, &_storedValue))
    {
        _storedValue = std::numeric_limits<int32_t>::min();
        _currentValue = _defaultValue;
    }
    else
    {
        _currentValue = _storedValue;
    }
}

void IntSetting::setDefault()
{
    if (_currentIsNvm)
    {
        NVSManager::EraseKey(_keyName);
    }
    else
    {
        _currentValue = _defaultValue;
        if (_storedValue != _currentValue)
        {
            NVSManager::EraseKey(_keyName);
        }
    }
}

EError IntSetting::setStringValue(char *s)
{
    s = trim(s);
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }
    char *endptr;
    int32_t convertedValue = strtol(s, &endptr, 10);
    if (endptr == s || *endptr != '\0')
    {
        return EError::BadNumberFormat;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue)
    {
        return EError::NumberRange;
    }

    // If we don't see the NVM state, we have to make this the live value:
    if (!_currentIsNvm)
    {
        _currentValue = convertedValue;
    }

    if (_storedValue != convertedValue)
    {
        if (convertedValue == _defaultValue)
        {
            NVSManager::EraseKey(_keyName);
        }
        else
        {
            if (NVSManager::WriteInt(_keyName, convertedValue) != EError::Ok)
            {
                return EError::NvsSetFailed;
            }
            _storedValue = convertedValue;
        }
    }
    check(NULL);
    return EError::Ok;
}

const char *IntSetting::getDefaultString()
{
    static char strval[32];
    sprintf(strval, "%d", _defaultValue);
    return strval;
}

const char *IntSetting::getStringValue()
{
    static char strval[32];

    int currentSettingValue;
    if (_currentIsNvm)
    {
        if (std::numeric_limits<int32_t>::min() == _storedValue)
        {
            currentSettingValue = _defaultValue;
        }
        else
        {
            currentSettingValue = _storedValue;
        }
    }
    else
    {
        currentSettingValue = get();
    }

    sprintf(strval, "%d", currentSettingValue);
    return strval;
}
