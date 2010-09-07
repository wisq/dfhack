/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef BASETYPES_H_INCLUDED
#define BASETYPES_H_INCLUDED

#include "DFPragma.h"
#include "DFExport.h"
#include "DFError.h"

namespace DFHack
{
    class Offset;
    class Address
    {
        bool valid : 1;
        uint32_t value;
    public:
        Address()
        {
            valid = false;
            value = 0xDEADBEEF;
        }
        Address(uint32_t val)
        {
            valid = true;
            value = val;
        }
        Address(const Address & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
        }
        inline bool isValid()
        {
            return valid;
        };
        operator uint32_t()
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            return value;
        }
        Address & operator=(const Address & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
            return *this;
        }
        Address & operator=(const uint32_t rhs)
        {
            valid = true;
            value = rhs;
            return *this;
        }

        template <class T>
        Address operator + (const T & offs)
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            return Address(value + offs);
        };

        template <class T>
        Address & operator += (const T & offs)
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            value += offs;
        };

        template <class T>
        Address operator - (const T & offs)
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            return Address(value - offs);
        };

        template <class T>
        Address & operator -= (const T & offs)
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            value += offs;
        };
    };

    class Offset
    {
        bool valid : 1;
        int32_t value;
    public:
        Offset()
        {
            valid = false;
            value = 0xDEADBEEF;
        }
        Offset(int32_t val)
        {
            valid = true;
            value = val;
        }
        Offset(const Offset & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
        }
        bool isValid()
        {
            return valid;
        };
        operator int32_t()
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("Address", "structural" );
            return value;
        }
        Offset & operator=(const Offset & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
            return *this;
        }
        Offset & operator=(const int32_t rhs)
        {
            valid = true;
            value = rhs;
            return *this;
        }
    };
    class HexValue
    {
        bool valid : 1;
        uint32_t value;
    public:
        HexValue()
        {
            valid = false;
            value = 0xDEADBEEF;
        }
        HexValue(uint32_t val)
        {
            valid = true;
            value = val;
        }
        HexValue(const HexValue & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
        }
        inline bool isValid()
        {
            return valid;
        };
        operator uint32_t()
        {
            if(!valid)
                throw Error::UnsetMemoryDefinition("HexValue", "structural" );
            return value;
        }
        HexValue & operator=(const HexValue & rhs)
        {
            valid = rhs.valid;
            value = rhs.value;
            return *this;
        }
        HexValue & operator=(const uint32_t rhs)
        {
            valid = true;
            value = rhs;
            return *this;
        }
    };
    class BaseString
    {
        bool valid : 1;
        std::string value;
    };

}// namespace DFHack
#endif // BASETYPES_H_INCLUDED
