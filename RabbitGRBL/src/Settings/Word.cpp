#include "Word.h"

Word::Word(ERabbitGRBLItemType type, const char *description, const char *grblName, const char *fullName)
    : _description(description), _grblName(grblName), _fullName(fullName), _type(type) {}
