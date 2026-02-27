#pragma once

#include "Setting.h"

class FlagSetting : public Setting
{
private:
    bool _defaultValue;
    int8_t _storedValue;
    bool _currentValue;

public:
    FlagSetting(const char *description,
                ERabbitGRBLItemType type,
                const char *grblName,
                const char *name,
                bool defVal,
                bool (*checker)(char *));
    FlagSetting(ERabbitGRBLItemType type, const char *grblName, const char *name, bool defVal, bool (*checker)(char *) = NULL) : FlagSetting(NULL, type, grblName, name, defVal, checker) {}

    void load();
    void setDefault();

    // There are no Flag settings in WebUI
    // The booleans are expressed as Enums
    EError setStringValue(char *value);
    const char *getCompatibleValue();
    const char *getStringValue();
    const char *getDefaultString();

    bool get() { return _currentValue; }
};
