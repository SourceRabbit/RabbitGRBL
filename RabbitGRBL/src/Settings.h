#pragma once

#include <map>

#include "Connection/MessageSender/MessageSender.h"
#include "NVS/NVSManager.h"

// Initialize the configuration subsystem
void settings_init();

// Define settings restore bitflags.
enum SettingsRestore
{
    Defaults = bit(0),
    Parameters = bit(1),
    StartupLines = bit(2),
    // BuildInfo = bit(3), // Obsolete
    Wifi = bit(4),
    All = 0xff,
};

// Restore subsets of settings to default values
void settings_restore(uint8_t restore_flag);

// Command::List is a linked list of all settings,
// so common code can enumerate them.
class Command;
// extern Command *CommandsList;

// This abstract class defines the generic interface that
// is used to set and get values for all settings independent
// of their underlying data type.  The values are always
// represented as human-readable strings.  This generic
// interface is used for managing settings via the user interface.

// Derived classes implement these generic functions for different
// kinds of data.  Code that accesses settings should use only these
// generic functions and should not use derived classes directly.

enum
{
    NO_AXIS = 255,
};
typedef enum : uint8_t
{
    GRBL = 1, // Classic GRBL settings like $100
    EXTENDED, // Settings added by early versions of Grbl_Esp32
    GRBLCMD,  // Non-persistent GRBL commands like $H
} type_t;

typedef uint8_t axis_t;

class Word
{
protected:
    const char *_description;
    const char *_grblName;
    const char *_fullName;
    type_t _type;

public:
    Word(type_t type, const char *description, const char *grblName, const char *fullName);
    type_t getType() { return _type; }
    const char *getName() { return _fullName; }
    const char *getGrblName() { return _grblName; }
    const char *getDescription() { return _description; }
};

class Command : public Word
{
protected:
    Command *link; // linked list of setting objects
    bool (*_cmdChecker)();

public:
    static Command *List;
    Command *next() { return link; }

    ~Command() {}
    Command(const char *description, type_t type, const char *grblName, const char *fullName, bool (*cmdChecker)());

    virtual EError action(char *value) = 0;
};

class Setting : public Word
{
private:
protected:
    // group_t _group;
    axis_t _axis = NO_AXIS;
    Setting *link; // linked list of setting objects

    bool (*_checker)(char *);
    const char *_keyName;

public:
    static Setting *List;
    Setting *next() { return link; }

    EError check(char *s);

    ~Setting() {}
    // Setting(const char *description, group_t group, const char * grblName, const char* fullName, bool (*checker)(char *));
    Setting(const char *description, type_t type, const char *grblName, const char *fullName, bool (*checker)(char *));
    axis_t getAxis() { return _axis; }
    void setAxis(axis_t axis) { _axis = axis; }

    // load() reads the backing store to get the current
    // value of the setting.  This could be slow so it
    // should be done infrequently, typically once at startup.
    virtual void load() {};
    virtual void setDefault() {};

    virtual EError setStringValue(char *value) = 0;
    EError setStringValue(String s) { return setStringValue(s.c_str()); }
    virtual const char *getStringValue() = 0;
    virtual const char *getCompatibleValue() { return getStringValue(); }
    virtual const char *getDefaultString() = 0;
};

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
               type_t type,
               const char *grblName,
               const char *name,
               int32_t defVal,
               int32_t minVal,
               int32_t maxVal,
               bool (*checker)(char *),
               bool currentIsNvm = false);

    IntSetting(type_t type,
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

class AxisMaskSetting : public Setting
{
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;

public:
    AxisMaskSetting(const char *description,
                    type_t type,
                    const char *grblName,
                    const char *name,
                    int32_t defVal,
                    bool (*checker)(char *));

    AxisMaskSetting(
        type_t type, const char *grblName, const char *name, int32_t defVal, bool (*checker)(char *) = NULL) : AxisMaskSetting(NULL, type, grblName, name, defVal, checker) {}

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getCompatibleValue();
    const char *getStringValue();
    const char *getDefaultString();

    int32_t get() { return _currentValue; }
};

class Coordinates
{
private:
    float _currentValue[MAX_N_AXIS];
    const char *_name;

public:
    Coordinates(const char *name) : _name(name) {}

    const char *getName() { return _name; }
    bool load();
    void setDefault()
    {
        float zeros[MAX_N_AXIS] = {
            0.0,
        };
        set(zeros);
    };
    // Copy the value to an array
    void get(float *value) { memcpy(value, _currentValue, sizeof(_currentValue)); }
    // Return a pointer to the array
    const float *get() { return _currentValue; }
    void set(float *value);
};

extern Coordinates *coords[CoordIndex::End];

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
                 type_t type,
                 const char *grblName,
                 const char *name,
                 float defVal,
                 float minVal,
                 float maxVal,
                 bool (*checker)(char *));

    FloatSetting(type_t type,
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
                  type_t type,
                  const char *grblName,
                  const char *name,
                  const char *defVal,
                  int min,
                  int max,
                  bool (*checker)(char *));

    StringSetting(
        type_t type, const char *grblName, const char *name, const char *defVal, bool (*checker)(char *) = NULL) : StringSetting(NULL, type, grblName, name, defVal, 0, 0, checker) {};

    void load();
    void setDefault();
    EError setStringValue(char *value);
    const char *getStringValue();
    const char *getDefaultString();

    const char *get() { return _currentValue.c_str(); }
};
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
                type_t type,
                const char *grblName,
                const char *name,
                int8_t defVal,
                enum_opt_t *opts,
                bool (*checker)(char *));

    EnumSetting(type_t type,
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

class FlagSetting : public Setting
{
private:
    bool _defaultValue;
    int8_t _storedValue;
    bool _currentValue;

public:
    FlagSetting(const char *description,
                type_t type,
                const char *grblName,
                const char *name,
                bool defVal,
                bool (*checker)(char *));
    FlagSetting(type_t type, const char *grblName, const char *name, bool defVal, bool (*checker)(char *) = NULL) : FlagSetting(NULL, type, grblName, name, defVal, checker) {}

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

class AxisSettings
{
public:
    const char *name;
    FloatSetting *steps_per_mm;
    FloatSetting *max_rate;
    FloatSetting *acceleration;
    FloatSetting *max_travel;
    FloatSetting *run_current;
    FloatSetting *hold_current;
    FloatSetting *home_mpos;
    IntSetting *microsteps;
    FloatSetting *backlash;

    AxisSettings(const char *axisName);
};

class GrblCommand : public Command
{
private:
    EError (*_action)(const char *);

public:
    GrblCommand(const char *grblName,
                const char *name,
                EError (*action)(const char *),
                bool (*cmdChecker)()) : Command(NULL, GRBLCMD, grblName, name, cmdChecker),
                                        _action(action) {}

    EError action(char *value);
};

template <typename T>
class FakeSetting
{
private:
    T _value;

public:
    FakeSetting(T value) : _value(value) {}

    T get() { return _value; }
};
