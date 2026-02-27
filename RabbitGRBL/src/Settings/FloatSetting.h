#pragma once

#include "Setting.h"

class FloatSetting : public Setting
{
private:
    float _defaultValue;
    float _currentValue;
    float _storedValue;
    float _minValue;
    float _maxValue;

public:
    FloatSetting(const char *description,
                 ERabbitGRBLItemType type,
                 const char *grblName,
                 const char *name,
                 float defVal,
                 float minVal,
                 float maxVal,
                 bool (*checker)(char *));

    FloatSetting(ERabbitGRBLItemType type,
                 const char *grblName,
                 const char *name,
                 float defVal,
                 float minVal,
                 float maxVal,
                 bool (*checker)(char *) = NULL) : FloatSetting(NULL, type, grblName, name, defVal, minVal, maxVal, checker) {}

    void load();
    void setDefault();

    // There are no Float settings in WebUI
    EError setStringValue(char *value);
    const char *getStringValue();
    const char *getDefaultString();

    float get() { return _currentValue; }
};
