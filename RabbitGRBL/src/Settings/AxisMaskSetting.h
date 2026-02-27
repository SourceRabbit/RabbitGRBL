#pragma once

#include "Setting.h"

class AxisMaskSetting : public Setting
{
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;

public:
    AxisMaskSetting(const char *description,
                    ERabbitGRBLItemType type,
                    const char *grblName,
                    const char *name,
                    int32_t defVal,
                    bool (*checker)(char *));

    AxisMaskSetting(
        ERabbitGRBLItemType type, const char *grblName, const char *name, int32_t defVal, bool (*checker)(char *) = NULL) : AxisMaskSetting(NULL, type, grblName, name, defVal, checker) {}

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getCompatibleValue();
    const char *getStringValue();
    const char *getDefaultString();

    int32_t get() { return _currentValue; }
};
