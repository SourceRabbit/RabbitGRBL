#include "../Grbl.h"

EnumSetting::EnumSetting(const char *description,
                         ERabbitGRBLItemType type,
                         const char *grblName,
                         const char *name,
                         int8_t defVal,
                         enum_opt_t *opts,
                         bool (*checker)(char *) = NULL) : Setting(description, type, grblName, name, checker),
                                                           _defaultValue(defVal), _options(opts) {}

void EnumSetting::load()
{
    // Read the stored enum value (int8) from NVS
    if (!NVSManager::ReadInt8(_keyName, &_storedValue))
    {
        _storedValue = -1;
        _currentValue = _defaultValue;
    }
    else
    {
        _currentValue = _storedValue;
    }
}

void EnumSetting::setDefault()
{
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue)
    {
        NVSManager::EraseKey(_keyName);
    }
}

// For enumerations, we allow the value to be set
// either with the string name or the numeric value.
// This is necessary for WebUI, which uses the number
// for setting.
EError EnumSetting::setStringValue(char *s)
{
    s = trim(s);
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }
    enum_opt_t::iterator it = _options->find(s);
    if (it == _options->end())
    {
        // If we don't find the value in keys, look for it in the numeric values

        // Disallow empty string
        if (!s || !*s)
        {
            return EError::BadNumberFormat;
        }
        char *endptr;
        uint8_t num = strtol(s, &endptr, 10);
        // Disallow non-numeric characters in string
        if (*endptr)
        {
            return EError::BadNumberFormat;
        }
        for (it = _options->begin(); it != _options->end(); it++)
        {
            if (it->second == num)
            {
                break;
            }
        }
        if (it == _options->end())
        {
            return EError::BadNumberFormat;
        }
    }
    _currentValue = it->second;
    if (_storedValue != _currentValue)
    {
        if (_currentValue == _defaultValue)
        {
            NVSManager::EraseKey(_keyName);
        }
        else
        {
            if (NVSManager::WriteInt8(_keyName, _currentValue) != EError::Ok)
            {
                return EError::NvsSetFailed;
            }
            _storedValue = _currentValue;
        }
    }
    check(NULL);
    return EError::Ok;
}

const char *EnumSetting::enumToString(int8_t value)
{
    for (enum_opt_t::iterator it = _options->begin(); it != _options->end(); it++)
    {
        if (it->second == value)
        {
            return it->first;
        }
    }
    return "???";
}
const char *EnumSetting::getDefaultString()
{
    return enumToString(_defaultValue);
}
const char *EnumSetting::getStringValue()
{
    return enumToString(get());
}
