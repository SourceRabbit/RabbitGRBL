#include "../Grbl.h"

StringSetting::StringSetting(const char *description,
                             ERabbitGRBLItemType type,
                             const char *grblName,
                             const char *name,
                             const char *defVal,
                             int min,
                             int max,
                             bool (*checker)(char *)) : Setting(description, type, grblName, name, checker)
{
    _defaultValue = defVal;
    _currentValue = defVal;
    _minLength = min;
    _maxLength = max;
};

void StringSetting::load()
{
    // First call with NULL buffer to get the required length
    size_t len = 0;
    if (!NVSManager::ReadString(_keyName, NULL, &len))
    {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    char buf[len];
    if (!NVSManager::ReadString(_keyName, buf, &len))
    {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    _storedValue = String(buf);
    _currentValue = _storedValue;
}

void StringSetting::setDefault()
{
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue)
    {
        NVSManager::EraseKey(_keyName);
    }
}

EError StringSetting::setStringValue(char *s)
{
    if (_minLength && _maxLength && (strlen(s) < _minLength || strlen(s) > _maxLength))
    {
        return EError::BadNumberFormat;
    }
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }
    _currentValue = s;
    if (_storedValue != _currentValue)
    {
        if (_currentValue == _defaultValue)
        {
            NVSManager::EraseKey(_keyName);
            _storedValue = _defaultValue;
        }
        else
        {
            if (NVSManager::WriteString(_keyName, _currentValue.c_str()) != EError::Ok)
            {
                return EError::NvsSetFailed;
            }
            _storedValue = _currentValue;
        }
    }
    check(NULL);
    return EError::Ok;
}

const char *StringSetting::getDefaultString()
{
    return _defaultValue.c_str();
}

const char *StringSetting::getStringValue()
{
    return get();
}
