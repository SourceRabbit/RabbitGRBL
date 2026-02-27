#include "../Grbl.h"

AxisMaskSetting::AxisMaskSetting(const char *description,
                                 ERabbitGRBLItemType type,
                                 const char *grblName,
                                 const char *name,
                                 int32_t defVal,
                                 bool (*checker)(char *) = NULL) : Setting(description, type, grblName, name, checker),
                                                                   _defaultValue(defVal), _currentValue(defVal) {}

void AxisMaskSetting::load()
{
    // Read the stored axis mask value from NVS
    if (!NVSManager::ReadInt(_keyName, &_storedValue))
    {
        _storedValue = -1;
        _currentValue = _defaultValue;
    }
    else
    {
        _currentValue = _storedValue;
    }
}

void AxisMaskSetting::setDefault()
{
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue)
    {
        NVSManager::EraseKey(_keyName);
    }
}

EError AxisMaskSetting::setStringValue(char *s)
{
    s = trim(s);
    EError err = check(s);
    if (err != EError::Ok)
    {
        return err;
    }
    int32_t convertedValue;
    char *endptr;
    if (*s == '\0')
    {
        convertedValue = 0;
    }
    else
    {
        convertedValue = strtol(s, &endptr, 10);
        if (endptr == s || *endptr != '\0')
        {
            // Try to convert as an axis list
            convertedValue = 0;
            auto axisNames = String("XYZABC");
            while (*s)
            {
                int index = axisNames.indexOf(toupper(*s++));
                if (index < 0)
                {
                    return EError::BadNumberFormat;
                }
                convertedValue |= bit(index);
            }
        }
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
            if (NVSManager::WriteInt(_keyName, _currentValue) != EError::Ok)
            {
                return EError::NvsSetFailed;
            }
            _storedValue = _currentValue;
        }
    }
    check(NULL);
    return EError::Ok;
}

const char *AxisMaskSetting::getCompatibleValue()
{
    static char strval[32];
    sprintf(strval, "%d", get());
    return strval;
}

static char *maskToString(uint32_t mask, char *strval)
{
    char *s = strval;
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        if (mask & bit(i))
        {
            *s++ = "XYZABC"[i];
        }
    }
    *s = '\0';
    return strval;
}

const char *AxisMaskSetting::getDefaultString()
{
    static char strval[32];
    return maskToString(_defaultValue, strval);
}

const char *AxisMaskSetting::getStringValue()
{
    static char strval[32];
    return maskToString(get(), strval);
}
