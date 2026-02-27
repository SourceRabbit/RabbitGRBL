#pragma once

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
