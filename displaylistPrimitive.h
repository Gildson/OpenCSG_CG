// OpenCSG - library for image-based CSG rendering for OpenGL
// Copyright (C) 2002-2022, Florian Kirsch,
// Hasso-Plattner-Institute at the University of Potsdam, Germany
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

//
// displaylistPrimitive.h 
//
// example for a primitive which renders itself using a display list
//

#ifndef __OpenCSG__displaylistprimitive_h__
#define __OpenCSG__displaylistprimitive_h__

#include <opencsg.h>

namespace OpenCSG {

    class DisplayListPrimitive : public Primitive {
    public:
        /// An object of this class contains the OpenGL id of a display
        /// list that is compiled by the application. render() just invokes
        /// this display list. 
        /// Operation and convexity are just forwarded to the base Primitive class.
        DisplayListPrimitive(unsigned int displayListId_, Operation, unsigned int convexity);

        /// Sets the display list id
        void setDisplayListId(unsigned int);
        /// Returns the display list id
        unsigned int getDisplayListId() const;

        /// Calls the display list.
        virtual void render();

    private:
        unsigned int mDisplayListId;
    };

} // namespace OpenCSG

#endif
