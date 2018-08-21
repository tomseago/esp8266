#include "tagged_buffer.h"

#include "log.h"

TaggedBuffer::TaggedBuffer(size_t initialCap) :
    len(0),
    capacity(0)
{
    data = (uint8_t*)malloc(initialCap);
    if (data)
    {
        capacity = initialCap;
        canGrow = true;
    }
    pCursor = data;
}

TaggedBuffer::TaggedBuffer(uint8_t* data, size_t len) :
    data(data),
    len(len),
    capacity(len),
    canGrow(false),
    pCursor(data)
{

}

TaggedBuffer::~TaggedBuffer()
{
    if (canGrow && data)
    {
        // We allocated it
        free(data);
    }
}

size_t
TaggedBuffer::write(uint8_t tag, uint8_t c)
{
    // 0 is not a valid tag
    if (!tag) return 0;

    if (!ensureSpace(3))
    {
        return 0;
    }

    *pCursor++ = tag;
    *pCursor++ = TypeUInt8;
    *pCursor++ = c;

    // Always force the length after a write because any
    // old data is not on proper boundaries now
    len = pCursor - data;

    return 3;
}


size_t
TaggedBuffer::write(uint8_t tag, uint16_t val)
{
    // 0 is not a valid tag
    if (!tag) return 0;

    if (!ensureSpace(4))
    {
        return 0;
    }

    *pCursor++ = tag;
    *pCursor++ = TypeUInt16;
    *pCursor++ = (uint8_t)((val >>  8) & 0x00ff);
    *pCursor++ = (uint8_t)((val      ) & 0x00ff);

    // Always force the length after a write because any
    // old data is not on proper boundaries now
    len = pCursor - data;

    return 4;
}


size_t
TaggedBuffer::write(uint8_t tag, uint32_t val)
{
    // 0 is not a valid tag
    if (!tag) return 0;

    if (!ensureSpace(6))
    {
        return 0;
    }

    *pCursor++ = tag;
    *pCursor++ = TypeUInt32;
    *pCursor++ = (uint8_t)((val >> 24) & 0x00ff);
    *pCursor++ = (uint8_t)((val >> 16) & 0x00ff);
    *pCursor++ = (uint8_t)((val >>  8) & 0x00ff);
    *pCursor++ = (uint8_t)((val      ) & 0x00ff);

    // Always force the length after a write because any
    // old data is not on proper boundaries now
    len = pCursor - data;

    return 6;
}


size_t
TaggedBuffer::write(uint8_t tag, const char* szStr)
{
    // 0 is not a valid tag
    if (!tag) return 0;

    if (!szStr) return 0;

    int sLen = strlen(szStr);

    // TODO: For long strings store a longer type with more bytes.
    // For now, everything is under 255 chars

    if (!ensureSpace(3+sLen))
    {
        return 0;
    }

    *pCursor++ = tag;
    *pCursor++ = TypeString;
    *pCursor++ = (uint8_t)sLen;
    if (sLen > 0)
    {
        memcpy(pCursor, szStr, sLen);
        pCursor += sLen;
    }

    // Always force the length after a write because any
    // old data is not on proper boundaries now
    len = pCursor - data;

    return 3+sLen;
}


size_t
TaggedBuffer::write(uint8_t tag, uint8_t* pBytes, uint16_t bytesLen)
{
    // 0 is not a valid tag
    if (!tag) return 0;

    if (!pBytes || !bytesLen) return 0;


    if (!ensureSpace(4+bytesLen))
    {
        return 0;
    }

    *pCursor++ = tag;
    *pCursor++ = TypeBytes;
    *pCursor++ = (uint8_t)(bytesLen >> 8);
    *pCursor++ = (uint8_t)(bytesLen & 0x00ff);

    memcpy(pCursor, pBytes, bytesLen);
    pCursor += bytesLen;

    // Always force the length after a write because any
    // old data is not on proper boundaries now
    len = pCursor - data;

    return 4 + bytesLen;
}

size_t
TaggedBuffer::writeNull()
{
    if (ensureSpace(1))
    {
        return 0;
    }
    *pCursor++ = 0;
    return 1;
}

///////


uint8_t
TaggedBuffer::nextTag()
{
    if (pCursor >= data+len)
    {
        return 0;
    }

    return *pCursor;
}

uint8_t
TaggedBuffer::nextType()
{
    if (pCursor+1 > data+len)
    {
        return 0;
    }

    return *(pCursor + 1);
}

uint8_t
TaggedBuffer::read(uint8_t* c)
{
    if (pCursor+3 > data+len)
    {
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeUInt8)
    {
        // Reading an invalid type
        return 0;
    }
    pCursor += 2;

    if (c)
    {
        *c = *pCursor;
    }
    pCursor++;

    return tag;
}


uint8_t
TaggedBuffer::read(uint16_t* pVal)
{
    if (pCursor+4 > data+len)
    {
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeUInt16)
    {
        // Reading an invalid type
        return 0;
    }
    pCursor += 2;

    if (pVal)
    {
        *pVal = (*(pCursor) << 8) + *(pCursor+1);
    }
    pCursor += 2;

    return tag;
}


uint8_t
TaggedBuffer::read(uint32_t* pVal)
{
    if (pCursor+6 > data+len)
    {
        Log.printf("---TB: Not enough data, avail=%p\n", data + len - pCursor);
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeUInt32)
    {
        // Reading an invalid type
        Log.printf("---TB: Invalid type=%d not %d\n", type, TypeUInt32);
        return 0;
    }
    pCursor += 2;

    if (pVal)
    {
        *pVal = (*(pCursor) << 24) + (*(pCursor+1) << 16) + (*(pCursor+2) << 8) + *(pCursor+3);
    }
    pCursor += 4;

    return tag;
}


uint8_t
TaggedBuffer::read(char** pszStr)
{
    if (pCursor+3 > data+len)
    {
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeString)
    {
        // Reading an invalid type
        return 0;
    }

    uint8_t sLen = *(pCursor+2);
    if (pCursor+3+sLen > data+len)
    {
        return 0;
    }

    // Commit to the read
    pCursor += 3;

    if (pszStr)
    {
        *pszStr = (char*)malloc(sLen + 1);
        if (!*pszStr)
        {
            pCursor += sLen;
            return 0;
        }

        memcpy(*pszStr, pCursor, sLen);
        (*pszStr)[sLen] = 0;
    }
    pCursor += sLen;

    return tag;
}


uint16_t
TaggedBuffer::bytesLength()
{
    if (pCursor+4 > data+len)
    {
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeBytes)
    {
        // Reading an invalid type
        return 0;
    }

    return (*(pCursor+2) << 8) + *(pCursor+3);
}

uint8_t
TaggedBuffer::readBytes(uint8_t* pBytes)
{
    if (pCursor+4 > data+len)
    {
        return 0;
    }

    uint8_t tag = *pCursor;
    uint8_t type = *(pCursor+1);

    if (type != TypeBytes)
    {
        // Reading an invalid type
        return 0;
    }

    uint16_t length = (*(pCursor+2) << 8) + *(pCursor+3);

    if (pCursor+4+length > data+len)
    {
        return 0;
    }

    // Now commit
    pCursor += 4;

    if (pBytes)
    {
        memcpy(pBytes, pCursor, length);
    }
    pCursor += length;

    return tag;
}

bool
TaggedBuffer::skip()
{
    uint8_t type = nextType();

    if (!type) return false; // no more tags

    switch(type)
    {
    case TypeUInt8:
        read((uint8_t*) NULL);
        break;

    case TypeString:
        read((char**) NULL);
        break;

    case TypeBytes:
        readBytes((uint8_t*) NULL);
        break;

    default:
        return false;
    }

    return true;
}

bool
TaggedBuffer::find(uint8_t tag)
{
    if (!tag) 
    {
        Log.printf("TB: Can not find NULL tag\n");
        return false;
    }

    // Special case for code that knows the order of tags
    if (nextTag() == tag)
    {
        Log.printf("TB: Special case on that tag already\n");
        return true;
    }

    // Go back to head and start skipping tags until we find it
    pCursor = data;

    uint8_t cur = nextTag();
    while(cur && cur != tag)
    {
        skip();
        cur = nextTag();
    }

    Log.printf("TB: find cur=%d tag=%d\n", cur, tag);
    return (cur == tag);
}

void
TaggedBuffer::toLog(bool newline)
{
    Log.printf("len=%d cap=%d ", len, capacity);
    
    for(int i=0; i<capacity; i++)
    {
        Log.printf("%02x ", data[i]);
    }
    if (newline)
    {
        Log.printf("\n");
    }
}

////////////////

bool
TaggedBuffer::ensureSpace(size_t amt)
{
    if (pCursor + amt < data + len)
    {
        // No worries
        return true;
    }

    // Does it fit in the capacity though?
    if (pCursor + amt < data + capacity)
    {
        return true;
    }

    // Doesn't fit in the capacity, but maybe we can grow it?
    if (!canGrow) return false;

    size_t toGrow = capacity / 2;
    if (toGrow == 0)
    {
        // Capacity was 0
        return false;
    }

    uint8_t* newData = (uint8_t*)realloc(data, capacity + toGrow);
    if (!newData)
    {
        return false;
    }
    data = newData;
    capacity += toGrow;
    return true;
}


///////////////////
void
TaggedBuffer::testCase()
{
    TaggedBuffer one(200);

    size_t written;

    one.write(3, (uint8_t)0xFF);
    one.write(1, "String");

    uint8_t byteData[] = { 1, 2, 3, 4};
    uint8_t bdl = 4;

    one.write(2, byteData, bdl);
    one.write(10, (uint16_t)0xfeed);
    one.write(20, (uint32_t)0xba5eba11);

    // Use this in a new buffer
    TaggedBuffer two(one.getData(), one.getLength());


    // Read some shit
    if (!two.find(1))
    {
        Log.printf("TBTC: Fail! Failed to find tag 1\n");
        return;
    }

    char* szOut = NULL;
    uint8_t tag = 0;

    tag = two.read(&szOut);
    if (tag != 1)
    {
        Log.printf("TBTC: Fail! Failed to read string\n");
        return;
    }

    if (strcmp(szOut, "String") != 0)
    {
        Log.printf("TBTC: Fail! String was wrong\n");
        return;
    }

    if (!two.find(3))
    {
        Log.printf("TBTC: Fail! Failed to find tag 3\n");
        return;
    }

    uint8_t val;
    tag = two.read(&val);
    if (val != 0xff)
    {
        Log.printf("TBTC: Fail! byte was wrong\n");
        return;
    }

    uint8_t bytesOut[bdl];
    two.skip();

    if (two.nextTag() != 2)
    {
        Log.printf("TBTC: Fail! Not at byte array\n");
        return;        
    }

    if (two.bytesLength() != bdl)
    {
        Log.printf("TBTC: Fail! Wrong bytes length\n");
        return;
    }
    tag = two.readBytes(bytesOut);

    for(int i=0; i<bdl; i++)
    {
        if (byteData[i] != bytesOut[i])
        {
            Log.printf("TBTC: Fail! Bad byte %d\n", i);
            return;
        }
    }

    two.find(10);
    uint16_t v16;
    if (!two.read(&v16) || v16 != 0xfeed)
    {
        Log.printf("TBTC: Fail! for uint16\n");
        return;
    }

    if (!two.find(20))
    {
        Log.printf("TBTC: Failed to find tag 20\n");
        return;        
    }

    uint32_t v32 = 1234;
    tag = two.read(&v32);
    if (!tag || v32 != 0xba5eba11)
    {
        Log.printf("TBTC: Fail! for uint32 tag=%d v32=%x\n", tag, v32);
        return;
    }

    Log.printf("TBTC: -------- PASS ---------\n");
}