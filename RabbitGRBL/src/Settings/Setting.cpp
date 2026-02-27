#include "../Grbl.h"

Setting *Setting::List = NULL;

Setting::Setting(
    const char *description, ERabbitGRBLItemType type, const char *grblName, const char *fullName, bool (*checker)(char *)) : Word(type, description, grblName, fullName),
                                                                                                                 _checker(checker)
{
    link = List;
    List = this;

    // NVS keys are limited to 15 characters, so if the setting name is longer
    // than that, we derive a 15-character name from a hash function
    size_t len = strlen(fullName);
    if (len <= 15)
    {
        _keyName = _fullName;
    }
    else
    {
        // This is Donald Knuth's hash function from Vol 3, chapter 6.4
        char *hashName = (char *)malloc(16);
        uint32_t hash = len;
        for (const char *s = fullName; *s; s++)
        {
            hash = ((hash << 5) ^ (hash >> 27)) ^ (*s);
        }
        sprintf(hashName, "%.7s%08x", fullName, hash);
        _keyName = hashName;
    }
}

EError Setting::check(char *s)
{
    if (sys.state != State::Idle && sys.state != State::Alarm)
    {
        return EError::IdleError;
    }
    if (!_checker)
    {
        return EError::Ok;
    }
    return _checker(s) ? EError::Ok : EError::InvalidValue;
}
