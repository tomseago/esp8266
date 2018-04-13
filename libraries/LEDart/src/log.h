#pragma once

#include <Print.h>
#include <Stream.h>

class DBLog: public Print {

public:
    DBLog();

    void setSerialEnabled(bool en);
    bool addPrint(Print *stream);
    void removePrint(Print *stream);

    void flush(void) override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

extern DBLog Log;