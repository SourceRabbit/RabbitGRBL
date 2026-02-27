#pragma once

#include "Setting.h"

class IntSetting : public Setting
{
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;
    int32_t _minValue;
    int32_t _maxValue;
    bool _currentIsNvm;

public:
    IntSetting(const char *description,
               ERabbitGRBLItemType type,
               const char *grblName,
               const char *name,
               int32_t defVal,
               int32_t minVal,
               int32_t maxVal,
               bool (*checker)(char *),
               bool currentIsNvm = false);

    IntSetting(ERabbitGRBLItemType type,
               const char *grblName,
               const char *name,
               int32_t defVal,
               int32_t minVal,
               int32_t maxVal,
               bool (*checker)(char *) = NULL,
               bool currentIsNvm = false) : IntSetting(NULL, type, grblName, name, defVal, minVal, maxVal, checker, currentIsNvm) {}

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getStringValue();
    const char *getDefaultString();

    int32_t get() { return _currentValue; }
};
