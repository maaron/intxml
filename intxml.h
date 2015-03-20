#pragma once

#include <exception>
#include <cctype>
#include <string>
#include <iterator>

// This file contains a set of low-level routines for parsing the various 
// constructs in an XML document.

namespace intxml
{
    class parsing_exception : public std::exception
    {
    public:
        template <typename chptr_t>
        parsing_exception(chptr_t&)
        {
        };
    };

    template <typename chptr_t>
    bool is_null(chptr_t& c)
    {
        return *c == 0;
    }

    template <typename chptr_t>
    class name_ptr
    {
        chptr_t c;
        bool end;

    public:
        typedef typename std::iterator_traits<chptr_t>::value_type char_type;

        name_ptr(chptr_t ptr) : c(ptr), end(false)
        {
            if (!std::isalpha(*c) && *c != '_') throw parsing_exception(c);
        }

        chptr_t& ptr() { return c; }

        char_type operator*()
        {
            return end ? 0 : *c;
        }

        name_ptr& operator++()
        {
            if (!end)
            {
                ++c;
                if (!std::isalpha(*c) &&
                    !std::isdigit(*c) &&
                    *c != '.' &&
                    *c != '-' &&
                    *c != '_' &&
                    *c != ':') end = true;
            }
            return *this;
        }

        name_ptr operator++(int)
        {
            name_ptr tmp(*this); 
            operator++(); 
            return tmp;
        }
    };

    template <typename chptr_t>
    class attribute_value_ptr
    {
        typedef typename std::iterator_traits<chptr_t>::value_type char_type;

        chptr_t c;
        enum { squote, dquote, end } state;

    public:
        attribute_value_ptr(chptr_t ptr) : c(ptr)
        {
            if (*c == '\'')
            {
                state = squote;
            }
            else if (*c == '"')
            {
                state = dquote;
            }
            else throw parsing_exception(c);

            operator++();
        }

        chptr_t& ptr() { return c; }

        char_type operator*()
        {
            if (state == end) return 0;
            else return *c;
        }

        attribute_value_ptr& operator++()
        {
            switch (state)
            {
            case squote:
                if (*++c == '\'')
                {
                    ++c;
                    state = end;
                }
                break;

            case dquote:
                if (*++c == '"')
                {
                    ++c;
                    state = end;
                }
                break;

            case end:
                break;
            }
            return *this;
        }

        attribute_value_ptr operator++(int)
        {
            attribute_value_ptr tmp(*this);
            operator++();
            return tmp;
        }
    };

    template <typename chptr_t>
    class text_ptr
    {
        typedef typename std::iterator_traits<chptr_t>::value_type char_type;

        chptr_t c;
        bool end;
        char_type entity;

        void lookahead()
        {
            if (*c == '&') parse_entity();
            else if (*c == '<') process_lt();
            else if (*c == '>') throw parsing_exception(c);
        }

        void parse_entity()
        {
            ++c;
            if (*c == '#')
            {
                ++c;
                entity = parse_character_reference(c);
            }
            else
            {
                entity = parse_entity_reference(c);
            }
        }

        void process_lt()
        {
            ++c;
            if (*c == '!')
            {
                ++c;
                parse<'-'>(c);
                parse_comment_dash_content_end(c);
            }
            else end = true;
        }

    public:
        text_ptr(chptr_t ptr) : c(ptr), end(false), entity(0)
        {
            lookahead();
        }

        chptr_t& ptr() { return c; }

        char_type operator*()
        {
            return 
                end ? 0 : 
                entity ? entity :
                *c;
        }

        text_ptr& operator++()
        {
            if (entity) entity = 0;
            else if (!end)
            {
                ++c;
                lookahead();
            }
            return *this;
        }

        text_ptr operator++(int)
        {
            text_ptr tmp(*this);
            operator++();
            return tmp;
        }
    };

    template <int ch, typename chptr_t>
    void parse(chptr_t& c)
    {
        if (*c != ch) throw parsing_exception(c);
        ++c;
    }

    template <typename chptr_t>
    int parse_character_reference(chptr_t& c)
    {
        if (*c == 'x')
        {
            ++c;
            return parse_hex_character_reference(c);
        }
        else return parse_decimal_character_reference(c);
    }

    template <typename chptr_t>
    int parse_decimal_character_reference(chptr_t& c)
    {
        int entity = 0;
        int digit;
        while ((digit = *c) != ';')
        {
            if (digit < 0x30 || digit > 0x39) throw parsing_exception(c);
            entity = entity * 10 + (digit & 0x0f);
            ++c;
        }
        ++c;
        return entity;
    }

    template <typename chptr_t>
    int parse_hex_character_reference(chptr_t& c)
    {
        int entity = 0;
        int digit;
        while ((digit = *c) != ';')
        {
            if (digit >= 0x30 && digit <= 0x39)
            {
                entity = entity * 16 + (digit & 0x0f);
            }
            else if ((digit >= 0x61 && digit <= 0x66) ||
                     (digit < 0x41 || digit > 0x46))
            {
                entity = entity * 16 + ((digit & 0x0f) + 9);
            }
            else throw parsing_exception(c);
            ++c;
        }
        ++c;
        return entity;
    }

    template <typename chptr_t>
    int parse_entity_reference(chptr_t& c)
    {
        int entity = '?';

        // This only supports the standard entities that don't require 
        // declaration- lt, gt, amp, apos, quot.
        if (*c == 'l')
        {
            ++c;
            parse<'t'>(c);
            entity = '<';
        }
        else if (*c == 'g')
        {
            ++c;
            parse<'t'>(c);
            entity = '>';
        }
        else if (*c == 'a')
        {
            if (*c == 'm')
            {
                ++c;
                parse<'p'>(c);
                entity = '&';
            }
            else if (*c == 'p')
            {
                ++c;
                parse<'o'>(c);
                parse<'s'>(c);
                entity = '\'';
            }
            else throw parsing_exception(c);
        }
        else if (*c == 'q')
        {
            ++c;
            parse<'u'>(c);
            parse<'o'>(c);
            parse<'t'>(c);
            entity = '"';
        }
        else throw parsing_exception(c);

        parse<';'>(c);
        return entity;
    }

    // Parses the "<" of a start tag.
    template <typename chptr_t>
    void parse_start_tag_lt(chptr_t& c)
    {
        parse<'<'>(c);
    }

    template <typename chptr_t>
    void parse_name(chptr_t& c)
    {
        if (!std::isalpha(*c) && *c != '_') throw parsing_exception(c);
        while (
            std::isalpha(*c) || 
            std::isdigit(*c) || 
            *c == '.' || 
            *c == '-' || 
            *c == '_' || 
            *c == ':')
        {
            if (is_null(c)) throw parsing_exception(c);
            ++c;
        }
    }

    // Parses tag name by calling the supplied handler with an object that provides transparent access to the characters in the name.
    template <typename chptr_t>
    void parse_start_tag_name(chptr_t& c)
    {
        parse_name(c);
    }

    template <typename chptr_t>
    bool parse_start_tag_name_end(chptr_t& c)
    {
        parse_name(c);
        parse_whitespace(c);
        return parse_start_tag_attribute_end(c);
    }

    template <typename chptr_t>
    bool parse_start_tag_attribute_end(chptr_t& c)
    {
        parse_attributes(c);
        return parse_start_tag_end(c);
    }

    // Parses the next attribute name by calling the supplied handler with an object that provides transparent access to the characters in the name.
    template <typename chptr_t>
    void parse_attribute_name(chptr_t& c)
    {
        parse_name(c);
    }

    // Parses the next attribute value by calling the supplied handler
    template <typename chptr_t>
    void parse_attribute_value(chptr_t& c)
    {
        auto start = *c;
        if (start != '\'' && start != '"') throw parsing_exception(c);

        do
        {
            if (is_null(c)) throw parsing_exception(c);
            ++c;
        } while (*c != start);

        ++c;
    }

    // Parses until the end of the start tag is found.  Returns true if the start tag ended with ">", and not "/>", i.e., returns true if the element is non-empty.
    template <typename chptr_t>
    bool parse_start_tag_end(chptr_t& c)
    {
        while (*c != '/' && *c != '>')
        {
            if (is_null(c)) throw parsing_exception(c);
            ++c;
        }

        if (*c == '>')
        {
            ++c;
            return true;
        }
        else
        {
            if (*c != '>') throw parsing_exception(c);
            ++c;
            return false;
        }
    }

    // Parses element content until either a child element or end tag is encountered.
    template <typename chptr_t>
    void parse_element_value(chptr_t& c)
    {
        while (*c != '<' && *c != '>')
        {
            if (is_null(c)) throw parsing_exception(c);
            ++c;
        }
    }

    template <typename chptr_t>
    void parse_end_tag(chptr_t& c)
    {
        parse<'<'>(c);
        parse_name(c);
        parse<'/'>(c);
        parse<'>'>(c);
    }

    template <typename chptr_t>
    void parse_xmldecl_content(chptr_t& c)
    {
        while (*c != '?') ++c;
    }

    template <typename chptr_t>
    void parse_xmldecl_end(chptr_t& c)
    {
        parse<'?'>(c);
        parse<'>'>(c);
    }

    template <typename chptr_t>
    void parse_xmldecl_content_end(chptr_t& c)
    {
        parse_xmldecl_content(c);
        parse_xmldecl_end(c);
    }

    template <typename chptr_t>
    void parse_pi_content_end(chptr_t& c)
    {
        while (true)
        {
            while (*c != '?')
            {
                if (is_null(c)) throw parsing_exception(c);
                ++c;
            }

            ++c;
            if (*c == '>') break;
        }
    }

    template <typename chptr_t>
    void parse_comment_dash_content_end(chptr_t& c)
    {
        parse<'-'>(c);

        while (true)
        {
            while (*c != '-')
            {
                if (is_null(c)) throw parsing_exception(c);
                ++c;
            }

            ++c;
            if (*c == '-')
            {
                ++c;
                break;
            }
        }

        parse<'>'>(c);
    }

    template <typename chptr_t>
    void parse_doctypedecl_content_end(chptr_t& c)
    {
        while (*c != '>')
        {
            if (is_null(c)) throw parsing_exception(c);
            ++c;
        }
        ++c;
    }

    template <typename chptr_t>
    void parse_prolog(chptr_t& c)
    {
        parse_whitespace(c);
        parse<'<'>(c);

        if (*c == '?')
        {
            ++c;
            parse_xmldecl_content_end(c);

            while (true)
            {
                parse_whitespace(c);
                parse<'<'>(c);

                ++c;
                if (*c == '?') parse_pi_content_end(c);
                else if (*c == '!')
                {
                    ++c;
                    if (*c == '-') parse_comment_dash_content_end(c);
                    else parse_doctypedecl_content_end(c);
                }
                else break;
            }
        }
    }

    // Parses until a non-whitespace character is found
    template <typename chptr_t>
    void parse_whitespace(chptr_t& c)
    {
        while (!is_null(c) && std::isspace(*c)) ++c;
    }

    template <typename chptr_t>
    void parse_element_content(chptr_t& c)
    {
        while (true)
        {
            if (parse_element_text(c))
            {
                parse_element_name_end(c);
            }
            else
            {
                parse<'/'>(c);
                parse_name(c);
                parse<'>'>(c);
                break;
            }
        }
    }

    // Returns true if more content remains in the element.  Upon returning 
    // false, c points to the beginning of the name of the closing tag 
    // (following the "</" sequence).  Upon returning true, c points to the
    // beginning of the tag name of the next child element.
    template <typename chptr_t>
    bool parse_element_text(chptr_t& c)
    {
        while (true)
        {
            parse_element_value(c);

            if (*c != '<') throw parsing_exception(c);
            ++c;

            if (*c == '/')
            {
                return false;
            }
            else if (*c == '!')
            {
                ++c;
                parse<'-'>(c);
                parse_comment_dash_content_end(c);
            }
            else return true;
        }
    }

    template <typename chptr_t>
    void parse_element_name_end(chptr_t& c)
    {
        parse_name(c);
        parse_whitespace(c);
        return parse_element_attribute_end(c);
    }

    template <typename chptr_t>
    void parse_element_attribute_end(chptr_t& c)
    {
        parse_attributes(c);
        if (parse_start_tag_end(c)) parse_element_content(c);
    }

    template <typename chptr_t>
    void parse_attribute(chptr_t& c)
    {
        parse_attribute_name(c);
        parse_whitespace(c);
        parse<'='>(c);
        parse_whitespace(c);
        parse_attribute_value(c);
    }

    template <typename chptr_t>
    void parse_attributes(chptr_t& c)
    {
        parse_whitespace(c);

        while (*c != '/' && *c != '>')
        {
            parse_attribute(c);
            parse_whitespace(c);
        }
    }

    template <typename chptr_t>
    void parse_doc(chptr_t& c)
    {
        parse_prolog(c);
        parse_element_name_end(c);
    }
}