#pragma once

#include <Arduino.h>

class TaggedBuffer {
public:
    static const uint8_t TypeUInt8 = 1;
    static const uint8_t TypeUInt16 = 2;
    static const uint8_t TypeUInt32 = 3;
    //     TypeInt8 = 4,
    //     TypeInt16 = 5,
    //     TypeInt32 = 6,
    //     TypeFloat = 7,
    static const uint8_t TypeString = 8;
    static const uint8_t TypeBytes = 9;

    // Use this when writing. A new buffer is created that can grow if needed
    TaggedBuffer(size_t initialCap=512);

    // Use this when reading from an existing buffer. If you write into a static
    // buffer the writes will fail if they would overflow and you probably want
    // to write a NULL at the end of your string of tagged values.
    TaggedBuffer(uint8_t* data, size_t len);
    ~TaggedBuffer();

    void wrote(size_t amt) { len = amt; }

    // The tag can not be 0, that is end of data marker
    size_t write(uint8_t tag, uint8_t c);
    size_t write(uint8_t tag, uint16_t val);
    size_t write(uint8_t tag, uint32_t val);
    size_t write(uint8_t tag, const char* szStr);
    size_t write(uint8_t tag, uint8_t* pBytes, uint16_t bytesLen);

    // Use this to add an invalid tag (a 0) to use as a marker for the end of the data
    size_t writeNull();

    uint8_t* getData() { return data; }
    size_t getLength() { return len; }
    size_t getCapacity() { return capacity; }

    uint8_t nextTag();
    uint8_t nextType();

    uint8_t read(uint8_t* c);
    uint8_t read(uint16_t* pVal);
    uint8_t read(uint32_t* pVal);
    uint8_t read(char** pszStr); // Creates a new string the caller must free

    uint16_t bytesLength(); // If the nextType is bytes this returns the length needed to hold it
    uint8_t readBytes(uint8_t* pBytes); // It is taken for granted that pBytes is long enough to hold the data (use bytesLength before calling this to make sure).

    // Advance to the next value if there is one
    bool skip();

    // This will find the value with the tag either in the current position or it will return to
    // the beginning and brute force try to find the first instance. Returns true if it
    // managed to find it.
    bool find(uint8_t tag);

    static void testCase();

    // Prints it to the log
    void toLog(bool newline=true);

protected:
    uint8_t* data;
    size_t len;
    size_t capacity;
    uint8_t* pCursor;

    bool canGrow;

    bool ensureSpace(size_t amt);
};
