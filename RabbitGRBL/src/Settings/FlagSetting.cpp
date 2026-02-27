#include "../Grbl.h"

FlagSetting::FlagSetting(const char *description,
                         ERabbitGRBLItemType type,
                         const char *grblName,
                         const char *name,
                         bool defVal,
                         bool (*checker)(char *) = NULL) : Setting(description, type, grblName, name, checker),
                                                           _defaultValue(defVal) {}

void FlagSetting::load()
{
    // Read the stored flag value (int8) from NVS
    if (!NVSManager::ReadInt8(_keyName, &_storedValue))
    {
        _storedValue = -1; // Neither well-formed false (0) nor true (1)
        _currentValue = _defaultValue;
    }
    else
    {
        _currentValue = !!_storedValue;
    }
}
void FlagSetting::setDefault()
{
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue)
    {
        NVSManager::EraseKey(_keyName);
    }
}

EError FlagSetting::setStringValue(char *s)
{
    s = trim(s);
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }
    _currentValue = (strcasecmp(s, "on") == 0) || (strcasecmp(s, "true") == 0) || (strcasecmp(s, "enabled") == 0) ||
                    (strcasecmp(s, "yes") == 0) || (strcasecmp(s, "1") == 0);
    // _storedValue is -1, 0, or 1
    // _currentValue is 0 or 1
    if (_storedValue != (int8_t)_currentValue)
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
const char *FlagSetting::getDefaultString()
{
    return _defaultValue ? "On" : "Off";
}
const char *FlagSetting::getStringValue()
{
    return get() ? "On" : "Off";
}
const char *FlagSetting::getCompatibleValue()
{
    return get() ? "1" : "0";
}
