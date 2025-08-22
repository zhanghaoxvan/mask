#pragma once

class str {
public:
    str(const char* s);
    const char* toCharArray();
private:
    const char* s;
};
