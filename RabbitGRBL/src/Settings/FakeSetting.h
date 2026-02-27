#pragma once

template <typename T>
class FakeSetting
{
private:
    T _value;

public:
    FakeSetting(T value) : _value(value) {}

    T get() { return _value; }
};
