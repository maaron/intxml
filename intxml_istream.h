#pragma once

#include <istream>
#include <iterator>

namespace intxml
{
    class istream_adapter : public std::iterator<std::input_iterator_tag, char>
    {
        std::streambuf* sbuf;

    public:
        istream_adapter(std::istream& istr) : sbuf(istr.rdbuf()) {}

        char operator*()
        {
            auto c = sbuf->sgetc();
            return c == std::streambuf::traits_type::eof() ?
                0 : c;
        }

        istream_adapter& operator++()
        {
            sbuf->sbumpc();
            return *this;
        }
    };
}