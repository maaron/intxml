#pragma once

#include <istream>
#include <iterator>
#include "intxml_line_counter.h"

namespace intxml
{
    class istream_adapter : public std::iterator<std::input_iterator_tag, char>
    {
        std::streambuf* sbuf;
        line_counter counter;

    public:
        istream_adapter(std::istream& istr) : sbuf(istr.rdbuf())
        {
            counter.update(sbuf->sgetc());
        }

        char operator*()
        {
            auto c = sbuf->sgetc();
            return c == std::streambuf::traits_type::eof() ?
                0 : c;
        }

        istream_adapter& operator++()
        {
            sbuf->sbumpc();
            counter.update(sbuf->sgetc());
            return *this;
        }

        int line() { return counter.line(); }
        
        int column() { return counter.column(); }
    };
}