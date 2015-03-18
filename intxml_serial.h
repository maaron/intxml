#pragma once

namespace intxml
{
    namespace serial
    {
        // Parsers for signed/unsigned, hex/decimal/etc. integers

        // Attribute read/write

        // Element read/write

        // Flexible element ordering support

        // Return an object that parses an element with the supplied tag 
        // name as a type "t".
        template <typename t>
        named_element<t> element_named(const char* name, t& element);
    }
}