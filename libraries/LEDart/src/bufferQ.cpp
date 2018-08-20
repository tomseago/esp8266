#include "bufferQ.h"

bool 
BufferQ::hasBuffers()
{
    return head;
}

uint8_t* 
BufferQ::peek(size_t* len)
{
    if (!head) 
    {
        if (len)
        {
            *len = 0;
        }
        return NULL;
    }

    if (len)
    {
        *len = head->len;
    }
    return head->data;
}

bool
BufferQ::pop()
{
    if (!head)
    {
        return false;
    }

    Buffer* pToKill = head;    
    head = head->next;
    if (head==NULL)
    {
        // That was the last, so clean up the tail
        tail = NULL;
    }

    free(pToKill->data);
    delete pToKill;

    return true;
}

// Copies something in using strndup to make sure it
// is NULL terminated
bool
BufferQ::pushCStr(char* str, size_t max)
{
    if (!str) return false;

    uint8_t* data = (uint8_t*)strndup((const char *)str, max);
    if (!data)
    {
        return false;
    }
    size_t len = strlen((const char *)data);

    if (!pushNoCopy(data, len))
    {
        free(data);
        return false;
    }

    return true;
}

// Copies the data exactly
bool
BufferQ::pushData(uint8_t* data, size_t len)
{
    if (!data) return false;

    uint8_t* copy = (uint8_t*)malloc(len);
    if (!copy) return false;

    if (!pushNoCopy(copy, len))
    {
        free(copy);
        return false;
    }

    return true;
}

bool
BufferQ::pushNoCopy(uint8_t* data, size_t len)
{
    if (!data) return false;

    Buffer* pNext = new Buffer();
    if (!pNext)
    {
        return false;
    }

    pNext->data = data;
    pNext->len = len;

    if (!head)
    {
        // First one in empty list
        head = pNext;
        tail = pNext;
    }
    else
    {
        // Respect the tail
        tail->next = pNext;
        tail = pNext;
    }
    return true;
}