//  $Id: item.cpp 5731 2010-08-08 22:54:04Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "items/item.hpp"

#include "graphics/irr_driver.hpp"
#include "karts/kart.hpp"
#include "utils/constants.hpp"
#include "utils/vec3.hpp"

Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           scene::IMesh* mesh, unsigned int item_id)
{
    setType(type);
    m_event_handler     = NULL;
    m_xyz               = xyz;
    m_deactive_time     = 0;
    // Sets heading to 0, and sets pitch and roll depending on the normal. */
    Vec3  hpr           = Vec3(0, normal);
    m_item_id           = item_id;
    m_original_type     = ITEM_NONE;
    m_collected         = false;
    m_time_till_return  = 0.0f;  // not strictly necessary, see isCollected()
    m_disappear_counter = m_type==ITEM_BUBBLEGUM 
                        ? stk_config->m_bubble_gum_counter
                        : -1 ;
    m_original_mesh    = mesh;
    m_node             = irr_driver->addMesh(mesh);
#ifdef DEBUG
    std::string debug_name("item: ");
    debug_name += m_type;
    m_node->setName(debug_name.c_str());
#endif

    m_node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);
    m_node->setPosition(xyz.toIrrVector());
    m_node->setRotation(hpr.toIrrHPR());
    m_node->grab();
}   // Item

//-----------------------------------------------------------------------------
/** Sets the type of this item, but also derived values, e.g. m_rotate.
 *  (bubblegums do not return).
 *  \param type New type of the item.
 */
void Item::setType(ItemType type)
{
    m_type   = type;
    m_rotate = type!=ITEM_BUBBLEGUM;
}   // setType

//-----------------------------------------------------------------------------
/** Changes this item to be a new type for a certain amount of time.
 *  \param type New type of this item.
 *  \param mesh Mesh to use to display this item.
 */
void Item::switchTo(ItemType type, scene::IMesh *mesh)
{
    m_original_type = m_type;
    setType(type);
    m_node->setMesh(mesh);
}   // switchTo

//-----------------------------------------------------------------------------
/** Switch  backs to the original item.
 */
void Item::switchBack()
{
    // If the item is not switched, do nothing. This can happen if a bubble
    // gum is dropped while items are switched - when switching back, this
    // bubble gum has no original type.
    if(m_original_type==ITEM_NONE)
        return;

    setType(m_original_type);
    m_original_type = ITEM_NONE;
    m_node->setMesh(m_original_mesh);
}   // switchBack

//-----------------------------------------------------------------------------
/** Removes an item.
 */
Item::~Item()
{
    irr_driver->removeNode(m_node);
    m_node->drop();
}   // ~Item

//-----------------------------------------------------------------------------
/** Resets before a race (esp. if a race is restarted).
 */
void Item::reset()
{
    m_collected         = false;
    m_time_till_return  = 0.0f;
    m_deactive_time     = 0.0f;
    m_disappear_counter = m_type==ITEM_BUBBLEGUM 
                        ? stk_config->m_bubble_gum_counter
                        : -1 ;
    if(m_original_type!=ITEM_NONE)
    {
        setType(m_original_type);
        m_original_type = ITEM_NONE;
    }
    
    m_node->setScale(core::vector3df(1,1,1));
}   // reset

//-----------------------------------------------------------------------------
/** Sets which karts dropped an item. This is used to avoid that a kart is
 *  affected by its own items.
 *  \param parent Kart that dropped the item.
 */
void Item::setParent(Kart* parent)
{
    m_event_handler = parent;
    m_deactive_time = 1.5f;
}   // setParent

//-----------------------------------------------------------------------------
/** Updated the item - rotates it, takes care of items coming back into
 *  the game after it has been collected.
 *  \param dt Time step size.
 */
void Item::update(float dt)
{
    if(m_deactive_time > 0) m_deactive_time -= dt;
    
    if(m_collected)
    {
        m_time_till_return -= dt;
        if(m_time_till_return<0)
        {
            m_collected=false;
            m_node->setScale(core::vector3df(1,1,1));

        }   // time till return <0 --> is fully visible again
        else if ( m_time_till_return <=1.0f )
        {
            // Make it visible by scaling it from 0 to 1:
            m_node->setVisible(true);
            m_node->setScale(core::vector3df(1,1,1)*(1-m_time_till_return));
        }   // time till return < 1
    }   // if collected
    else
    {   // not m_collected
        
        if(!m_rotate) return;
        // have it rotate
        Vec3 rotation(0, dt*M_PI, 0);
        core::vector3df r = m_node->getRotation();
        r.Y += dt*180.0f;
        if(r.Y>360.0f) r.Y -= 360.0f;
        m_node->setRotation(r);
        return;
    }   // not m_collected
}   // update

//-----------------------------------------------------------------------------
/** Is called when the item is hit by a kart.  It sets the flag that the item
 *  has been collected, and the time to return to the parameter. 
 *  \param t Time till the object reappears (defaults to 2 seconds).
 */
void Item::collected(const Kart *kart, float t)
{
    m_collected     = true;
    m_event_handler = kart;
    if(m_type==ITEM_BUBBLEGUM && m_disappear_counter>0)
    {
        m_disappear_counter --;
        // Deactivates the item for a certain amount of time. It is used to
        // prevent bubble gum from hitting a kart over and over again (in each
        // frame) by giving it time to drive away.
        m_deactive_time = 0.5f;
        // Set the time till reappear to -1 seconds --> the item will 
        // reappear immediately.
        m_time_till_return = -1;
    }
    else
    {
        // Note if the time is negative, in update the m_collected flag will
        // be automatically set to false again.
        m_time_till_return = t;
        m_node->setVisible(false);
    }
}   // isCollected

