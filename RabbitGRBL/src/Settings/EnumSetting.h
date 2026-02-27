#pragma once

#include <map>
#include "Setting.h"

struct cmp_str
{
    bool operator()(char const *a, char const *b) const { return strcasecmp(a, b) < 0; }
};
typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;

class EnumSetting : public Setting
{
private:
    int8_t _defaultValue;
    int8_t _storedValue;
    int8_t _currentValue;
    std::map<const char *, int8_t, cmp_str> *_options;
    const char *enumToString(int8_t value);

public:
    EnumSetting(const char *description,
                ERabbitGRBLItemType type,
                const char *grblName,
                const char *name,
                int8_t defVal,
                enum_opt_t *opts,
                bool (*checker)(char *));

    EnumSetting(ERabbitGRBLItemType type,
                const char *grblName,
                const char *name,
                int8_t defVal,
                enum_opt_t *opts,
                bool (*checker)(char *) = NULL) : EnumSetting(NULL, type, grblName, name, defVal, opts, checker) {}

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getStringValue();
    const char *getDefaultString();

    int8_t get() { return _currentValue; }
};
