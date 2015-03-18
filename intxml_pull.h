#pragma once

#include "intxml.h"
#include <boost/optional.hpp>

// This file implements a "pull" interface, similar to .NET System.Xml.XmlReader.

namespace intxml
{
    class pull_exception : public std::exception
    {
        template <typename chptr_t>
        pull_exception(chptr_t c)
        {
        }
    };

    template <typename chptr_t>
    class attribute
    {
        chptr_t c;

    public:
        attribute(chptr_t ptr) : c(ptr) {}

        name_ptr<chptr_t> name() { return name_ptr<chptr_t>(c); }

        attribute_value_ptr<chptr_t> value()
        {
            chptr_t cnew(c);
            parse_attribute_name(cnew);
            parse_whitespace(cnew);
            parse<'='>(cnew);
            parse_whitespace(cnew);
            return attribute_value_ptr<chptr_t>(cnew);
        }

        boost::optional<attribute> attrib()
        {
            chptr_t cnew(c);
            intxml::parse_attribute(cnew);
            intxml::parse_whitespace(cnew);
            if (*cnew != '/' && *cnew != '>') return attribute(cnew);
            else return boost::none;
        }

        boost::optional<element> child()
        {
            chptr_t cnew(c);
            if (intxml::parse_start_tag_attribute_end(cnew))
            {
                intxml::parse_element_text(cnew);
                return element(cnew);
            }
            else return boost::none;
        }

        boost::optional<element> sibling()
        {
            chptr_t cnew(c);
            intxml::parse_element_attribute_end(cnew);
            if (intxml::parse_element_text(cnew))
                return element(cnew);
            else return boost::none;
        }
    };

    template <typename chptr_t>
    class element
    {
        chptr_t c;

    public:
        element(chptr_t ptr) : c(ptr) {}

        boost::optional<attribute> attrib()
        {
            chptr_t cnew(c);
            intxml::parse_name(cnew);
            intxml::parse_whitespace(cnew);
            if (*cnew != '/' && *cnew != '>') return attribute(cnew);
            else return boost::none;
        }

        boost::optional<element> child()
        {
            chptr_t cnew(c);
            intxml::parse_name(cnew);
            if (intxml::parse_start_tag_name_end(cnew))
            {
                intxml::parse_element_text(cnew);
                return element(cnew);
            }
            else return boost::none;
        }

        boost::optional<element> sibling()
        {
            chptr_t cnew(c);
            intxml::parse_element_name_end(cnew);
            if (intxml::parse_element_text(cnew))
                return element(cnew);
            else return boost::none;
        }

        boost::optional<element> uncle()
        {
            chptr_t cnew(c);
            // TODO
        }
    };

    template <typename chptr_t>
    class document
    {
        chptr_t c;

    public:
        document(chptr_t ptr) : c(ptr)
        {
        }

        element<chptr_t> root()
        {
            intxml::parse_prolog(c);
            return element(c);
        }
    };

}