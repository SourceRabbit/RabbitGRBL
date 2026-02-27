#pragma once

#include "Word.h"
#include "../Diagnostics/Errors/EError.h"

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
    Setting(const char *description, ERabbitGRBLItemType type, const char *grblName, const char *fullName, bool (*checker)(char *));
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
