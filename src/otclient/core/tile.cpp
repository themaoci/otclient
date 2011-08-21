#include "tile.h"
#include "item.h"
#include "datmanager.h"
#include "map.h"
#include "game.h"
#include "localplayer.h"
#include <framework/graphics/fontmanager.h>

Tile::Tile()
{
    m_drawNextOffset = 0;
}

void Tile::draw(int x, int y)
{
    FontPtr font = g_fonts.getDefaultFont();

    m_drawNextOffset = 0;

    if(m_ground)
        m_ground->draw(x, y);

    for(auto it = m_itemsTop.rbegin(), end = m_itemsTop.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        const ThingAttributes& thingAttributes = thing->getAttributes();

        if(thingAttributes.alwaysOnTopOrder == 1) {
            thing->draw(x - m_drawNextOffset, y - m_drawNextOffset);
            //font->renderText("T1", Rect(x + 5, y+5, 100, 100));

            m_drawNextOffset += thingAttributes.drawNextOffset;
        }
    }

    for(auto it = m_itemsTop.rbegin(), end = m_itemsTop.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        const ThingAttributes& thingAttributes = thing->getAttributes();

        if(thingAttributes.alwaysOnTopOrder == 2) {
            thing->draw(x - m_drawNextOffset, y - m_drawNextOffset);
            //font->renderText("T2", Rect(x + 5, y+5, 100, 100));
            m_drawNextOffset += thingAttributes.drawNextOffset;
        }
    }

    for(auto it = m_itemsBottom.rbegin(), end = m_itemsBottom.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        const ThingAttributes& thingAttributes = thing->getAttributes();
        thing->draw(x - m_drawNextOffset, y - m_drawNextOffset);
        //font->renderText("B0", Rect(x + 5, y+5, 100, 100));
        m_drawNextOffset += thingAttributes.drawNextOffset;
    }

    for(auto it = m_creatures.rbegin(), end = m_creatures.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        thing->draw(x - m_drawNextOffset, y - m_drawNextOffset);
    }

    for(auto it = m_itemsTop.rbegin(), end = m_itemsTop.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        const ThingAttributes& thingAttributes = thing->getAttributes();

        if(thingAttributes.alwaysOnTopOrder == 3) {
            thing->draw(x, y);
            //font->renderText("T3", Rect(x + 5, y+5, 100, 100));
        }
    }

    for(auto it = m_effects.rbegin(), end = m_effects.rend(); it != end; ++it) {
        const ThingPtr& thing = *it;
        thing->draw(x - m_drawNextOffset, y - m_drawNextOffset);
    }
}

void Tile::addThing(ThingPtr thing, uint8 stackpos)
{
    if(!thing)
        return;

    if(thing->getPosition() == g_game.getLocalPlayer()->getPosition() + Position(-1, 0, 0) && thing->getAttributes().alwaysOnTop) {
        logDebug((int)thing->getId());
    }

    const ThingAttributes& thingAttributes = thing->getAttributes();

    if(thing->asItem()) {
        if(thingAttributes.group == THING_GROUP_GROUND)
            m_ground = thing;
        else {
            if(thingAttributes.alwaysOnTop)
                m_itemsTop.push_back(thing);
            else
                m_itemsBottom.push_back(thing);
        }
    }
    else if(thing->asCreature()) {
        m_creatures.push_back(thing);
    }
    else if(thing->asEffect()) {
        m_effects.push_back(thing);
    }
}

ThingPtr Tile::getThing(uint8 stackpos)
{
    if(stackpos == 0)
        return m_ground;
    --stackpos;

    if(stackpos < m_itemsTop.size())
        return m_itemsTop[stackpos];
    stackpos -= m_itemsTop.size();

    if(stackpos < m_creatures.size())
        return m_creatures[stackpos];
    stackpos -= m_creatures.size();

    if(stackpos < m_itemsBottom.size())
        return m_itemsBottom[stackpos];

    return ThingPtr();
}

void Tile::removeThing(uint8 stackpos)
{
    if(stackpos == 0) {
        m_ground.reset();
        return;
    }
    --stackpos;

    if(stackpos < m_itemsTop.size()) {
        m_itemsTop.erase(m_itemsTop.begin() + stackpos);
        return;
    }
    stackpos -= m_itemsTop.size();

    if(stackpos < m_creatures.size()) {
        m_creatures.erase(m_creatures.begin() + stackpos);
        return;
    }
    stackpos -= m_creatures.size();

    if(stackpos < m_itemsBottom.size()) {
        m_itemsBottom.erase(m_itemsBottom.begin() + stackpos);
        return;
    }

    logDebug("Invalid stackpos.");
}

void Tile::removeThingByPtr(ThingPtr thing)
{
    // Items
    if(thing->asItem()) {
        const ThingAttributes& thingAttributes = thing->getAttributes();

        if(!thingAttributes.alwaysOnTop) {
            for(auto it = m_itemsBottom.begin(), end = m_itemsBottom.end(); it != end; ++it) {
                if(*it == thing) {
                    m_itemsBottom.erase(it);
                    break;
                }
            }
        }
        else {
            for(auto it = m_itemsTop.begin(), end = m_itemsTop.end(); it != end; ++it) {
                if(*it == thing) {
                    m_itemsTop.erase(it);
                    break;
                }
            }
        }
    }

    // Creatures
    else if(thing->asCreature()) {
        for(auto it = m_creatures.begin(), end = m_creatures.end(); it != end; ++it) {
            if(*it == thing) {
                m_creatures.erase(it);
                break;
            }
        }
    }

    // Effects
    else if(thing->asEffect()) {
        for(auto it = m_effects.begin(), end = m_effects.end(); it != end; ++it) {
            if(*it == thing) {
                m_effects.erase(it);
                break;
            }
        }
    }
}

void Tile::clean()
{
    for(const ThingPtr& thing : m_creatures)
        g_map.removeCreatureById(thing->getId());

    m_itemsTop.clear();
    m_creatures.clear();
    m_itemsBottom.clear();
    m_effects.clear();
}

int Tile::getStackSize()
{
    int ret = 0;
    if(m_ground)
        ret++;
    ret += m_itemsBottom.size();
    ret += m_creatures.size();
    ret += m_itemsTop.size();
    return ret;
}
