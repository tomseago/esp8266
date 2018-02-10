#pragma once

#include <LEDArt.h>

class PyramidsGeometry : public LEDArtGeometry {
protected:
    NeoTopology<LEDART_TOPO_LAYOUT> topo;

public:
    PyraimdsGeometry(uint16_t width, uint16_t height);
    virtual uint16_t Map(uint16_t x, uint16_t y);    
};