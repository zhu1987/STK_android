//  $Id: track_object_manager.hpp 4308 2009-12-17 00:22:29Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#ifndef HEADER_TRACK_OBJECT_MANAGER_HPP
#define HEADER_TRACK_OBJECT_MANAGER_HPP

#include <vector>

class PhysicalObject;
class Track;
class TrackObject;
class Vec3;
class XMLNode;

/**
  * \ingroup tracks
  */
class TrackObjectManager
{
public:
    /**
      * The different type of track objects: physical objects, graphical 
      * objects (without a physical representation) - the latter might be
      * eye candy (to reduce work for physics), ...
      */
    enum TrackObjectType {TO_PHYSICAL, TO_GRAPHICAL};
    std::vector<TrackObject*> m_all_objects;
public:
         TrackObjectManager();
        ~TrackObjectManager();
    void add(const XMLNode &xml_node, const Track &track);
    void update(float dt);
    void handleExplosion(const Vec3 &pos, const PhysicalObject *mp) const;
    void reset();
    void init();
};   // class TrackObjectManager

#endif
