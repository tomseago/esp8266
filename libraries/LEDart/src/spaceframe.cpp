#include "spaceframe.h"
#include "node_config.h"

uint16_t
SFGeomSideGrid::map(uint16_t x, uint16_t y)
{
    uint8_t nodeId = NodeConfig.nodeId();

    if (nodeId >= 5 && nodeId <= 8)
    {
        // // Top panels, never do anything here
        return -1;
    }

    // Get the start of the panel
    uint16_t pIx = (nodeId-1) * SFMax;
    uint8_t panelPos = nodeId < 5 ? nodeId-1 : nodeId - 9;

    if (y==0 || y == 35)
    {
        // Top or bottom, x is the linear offset from the
        // front of the car back along the side

        // Is it on our panel?
        uint8_t pixPanelPos = x / 16;
        if (pixPanelPos != panelPos) {
            return -1;
        }

        // How far into the panel is it?
        uint8_t pixInteralPos = x % 16;

        if (y==35)
        {
            // From our bottom left corner
            return SFBotLeft + pixInteralPos;
        }

        // It's on the top, which is in two pieces
        if (pixInteralPos < 9)
        {
            return SFTopLeft - 1 - pixInteralPos;
        }

        // The right half of the top edge
        return SFMax - 1 - (pixInteralPos - 9);
    }

    // We only have two pixels on the edges, maybe it's one of them?
    uint8_t pixPanelPos = x / 2;
    if (pixPanelPos != panelPos) {
        return -1;
    }

    if (x % 2 != 0)
    {
        // The right side
        return SFBotRight + 34 - y;
    }

    // The left side
    return SFTopLeft + (y - 1);
}


uint16_t
SFGeomSideGrid::getWidth(int16_t y)
{
    if (y>0 && y<35) 
    {
        // Middle section
        return 8;
    }

    // Default
    return 4 * 16;
}


/////////////

uint16_t
SFGeomSideMirrored::map(uint16_t x, uint16_t y)
{
    uint8_t panelId = NodeConfig.nodeId() - 1;

    uint8_t panelPos = panelId / 4;
    // TODO: This isn't actually done, it's just pacedholder

    if (y==0 || y == 35)
    {
        // Top or bottom, x is the linear offset from the
        // front of the car back along the side

        // Is it on our panel?
        uint8_t pixPanelPos = x / 16;
        if (pixPanelPos != panelPos) {
            return -1;
        }

        // How far into the panel is it?
        uint8_t pixInteralPos = x % 16;

        if (y==35)
        {
            // From our bottom left corner
            return SFBotLeft + pixInteralPos;
        }

        // It's on the top, which is in two pieces
        if (pixInteralPos < 9)
        {
            return SFTopLeft - 1 - pixInteralPos;
        }

        // The right half of the top edge
        return SFMax - 1 - (pixInteralPos - 9);
    }

    // We only have two pixels on the edges, maybe it's one of them?
    uint8_t pixPanelPos = x / 2;
    if (pixPanelPos != panelPos) {
        return -1;
    }

    if (x % 2 != 0)
    {
        // The right side
        return SFBotRight + 34 - y;
    }

    // The left side
    return SFTopLeft + (y - 1);
}


uint16_t
SFGeomSideMirrored::getWidth(int16_t y)
{
    if (y == 17 || y == 18 || y == 53)
    {
        return 4 * 16;
    }

    // A skinny part
    return 8;
}


/////////////

uint16_t
SFGeomWrappedOver::map(uint16_t x, uint16_t y)
{
    uint8_t nodeId = NodeConfig.nodeId();

    //uint8_t panelCol = nodeId < 5 ? nodeId-1 : (nodeId < 9 ? nodeId - 5 : nodeId - 9);

    uint8_t panelRow = 2 - ((nodeId - 1) / 4);
    uint8_t pixPanelRow = y / 36;
    if (panelRow != pixPanelRow) return -1;

    uint8_t panelCol = (nodeId - 1) % 4;
    uint8_t yInPanel = y % 36;

    if (panelRow != 2)
    {
        // Invert the first two (back and top)
        yInPanel = 35 - yInPanel;
    }

    if (yInPanel==0 || yInPanel == 35)
    {
        // Top or bottom, x is the linear offset from the
        // front of the car back along the side

        // Is it on our panel?
        uint8_t pixPanelCol = x / 16;
        if (pixPanelCol != panelCol) {
            return -1;
        }

        // How far into the panel is it?
        uint8_t xInPanel = x % 16;

        if (yInPanel==35)
        {
            // From our bottom left corner
            return SFBotLeft + xInPanel;
        }

        // It's on the top, which is in two pieces
        if (xInPanel < 9)
        {
            return SFTopLeft - 1 - xInPanel;
        }

        // The right half of the top edge
        return SFMax - 1 - (xInPanel- 9);
    }

    // We only have two pixels on the edges, maybe it's one of them?
    uint8_t pixPanelCol = x / 2;
    if (pixPanelCol != panelCol) {
        return -1;
    }

    if (x % 2 != 0)
    {
        // The right side
        return SFBotRight + 34 - yInPanel;
    }

    // The left side
    return SFTopLeft + (yInPanel - 1);
}


uint16_t
SFGeomWrappedOver::getWidth(int16_t y)
{
    uint8_t yInPanel = y % 36;

    if (yInPanel>0 && yInPanel<35) 
    {
        // Middle section
        return 8;
    }

    // Default
    return 4 * 16;
}


/////////////


uint16_t
SFGeomWrappedLong::map(uint16_t x, uint16_t y)
{
    uint8_t panelId = NodeConfig.nodeId() - 1;

    //uint8_t panelCol = nodeId < 5 ? nodeId-1 : (nodeId < 9 ? nodeId - 5 : nodeId - 9);

    uint8_t panelRow = panelId % 4;
    uint8_t pixPanelRow = y / 18;
    if (panelRow != pixPanelRow) return -1;

    uint8_t panelCol = panelId / 4;

    uint8_t yInPanel = y % 18;

    if (yInPanel==0 || yInPanel == 17)
    {
        // Top or bottom

        // Is it on our panel?
        uint8_t pixPanelCol = x / 34;
        if (pixPanelCol != panelCol) {
            return -1;
        }

        // How far into the panel is it?
        uint8_t xInPanel = x % 34;

        if (yInPanel==0)
        {
            // From our "bottom left" corner once it's unfolded
            return SFBotLeft - 1 - xInPanel;
        }

        // It's on the right hand edge
        return SFBotRight + xInPanel;
    }

    // We only have two pixels on the edges, maybe it's one of them?
    uint8_t pixPanelCol = x / 2;
    if (pixPanelCol != panelCol) {
        return -1;
    }

    if (x % 2 != 0)
    {
        // The right side (aka the top, which is annoying because it's
        // split
        // SFTopLeft is actually on the left side, not our last pixel on
        // the top, but one past that so it collapses with the -1 term
        if (yInPanel < 10)
        {
            return 9 - yInPanel;
        }

        return SFMax - (yInPanel - 9);
    }

    // The left side (aka the bottom)
    return SFBotLeft + (yInPanel-1);
}


uint16_t
SFGeomWrappedLong::getWidth(int16_t y)
{
    uint8_t yInPanel = y % 18;

    if (yInPanel>0 && yInPanel<17) 
    {
        // Middle section
        return 6;
    }

    // Default is long edge
    return 34 * 3;
}

