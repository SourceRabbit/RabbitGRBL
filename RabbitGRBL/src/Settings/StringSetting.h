#pragma once

#include "Setting.h"

class StringSetting : public Setting
{
private:
    String _defaultValue;
    String _currentValue;
    String _storedValue;
    int _minLength;
    int _maxLength;
    void _setStoredValue(const char *s);

public:
    StringSetting(const char *description,
                  ERabbitGRBLItemType type,
                  const char *grblName,
                  const char *name,
                  const char *defVal,
                  int min,
                  int max,
                  bool (*checker)(char *));

    StringSetting(
        ERabbitGRBLItemType type, const char *grblName, const char *name, const char *defVal, bool (*checker)(char *) = NULL) : StringSetting(NULL, type, grblName, name, defVal, 0, 0, checker) {};

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getStringValue();
    const char *getDefaultString();

    const char *get() { return _currentValue.c_str(); }
};
