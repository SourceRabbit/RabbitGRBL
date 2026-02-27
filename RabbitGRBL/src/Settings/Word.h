#pragma once

#include <Arduino.h>
#include "../Core/ERabbitGRBLItemType.h"

typedef uint8_t axis_t;

class Word
{
protected:
    const char *_description;
    const char *_grblName;
    const char *_fullName;
    ERabbitGRBLItemType _type;

public:
    Word(ERabbitGRBLItemType type, const char *description, const char *grblName, const char *fullName);
    ERabbitGRBLItemType getType() { return _type; }
    const char *getName() { return _fullName; }
    const char *getGrblName() { return _grblName; }
    const char *getDescription() { return _description; }
};
