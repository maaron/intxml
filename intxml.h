#pragma once

#include <exception>
#include <cctype>
#include <string>

namespace intxml
{
    template <typename chptr_t>
    bool eof(chptr_t& c)
    {
        return std::char_traits<chptr_t>::eof() == *c;
    }

    class parsing_exception : public std::exception
    {
    public:
        template <typename chptr_t>
        parsing_exception(chptr_t&)
        {
        };
    };

    template <int ch, typename chptr_t>
    void parse(chptr_t& c)
    {
        if (*c != ch) throw parsing_exception(c);
        c++;
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
            if (eof(c)) throw parsing_exception(c);
            c++;
        }
    }

    // Parses tag name by calling the supplied handler with an object that provides transparent access to the characters in the name.
    template <typename chptr_t>
    void parse_start_tag_name(chptr_t& c)
    {
        parse_name(c);
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
            if (eof(c)) throw parsing_exception(c);
            c++;
        } while (*c != start);

        c++;
    }

    // Parses until the end of the start tag is found.  Returns true if the start tag ended with ">", and not "/>", i.e., returns true if the element is non-empty.
    template <typename chptr_t>
    bool parse_start_tag_end(chptr_t& c)
    {
        while (*c != '/' && *c != '>')
        {
            if (eof(c)) throw parsing_exception(c);
            c++;
        }

        if (*c++ == '>') return true;
        else
        {
            if (*c != '>') throw parsing_exception(c);
            c++;
            return false;
        }
    }

    // Parses element content until either a child element or end tag is encountered.
    template <typename chptr_t>
    void parse_element_value(chptr_t& c)
    {
        while (*c != '<' && *c != '>')
        {
            if (eof(c)) throw parsing_exception(c);
            c++;
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
        while (*c != '?') c++;
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
                if (eof(c)) throw parsing_exception(c);
                c++;
            }

            c++;
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
                if (eof(c)) throw parsing_exception(c);
                c++;
            }

            c++;
            if (*c == '-') break;
        }

        parse<'>'>(c);
    }

    template <typename chptr_t>
    void parse_doctypedecl_content_end(chptr_t& c)
    {
        while (*c != '>')
        {
            if (eof(c)) throw parsing_exception(c);
            c++;
        }
        c++;
    }

    template <typename chptr_t>
    void parse_prolog(chptr_t& c)
    {
        parse_whitespace(c);
        parse<'<'>(c);

        if (*c == '?')
        {
            c++;
            parse_xmldecl_content_end(c);

            while (true)
            {
                parse_whitespace(c);
                parse<'<'>(c);

                c++;
                if (*c == '?') parse_pi_content_end(c);
                else if (*c == '!')
                {
                    c++;
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
        while (!eof(c) && std::isspace(*c)) c++;
    }

    template <typename chptr_t>
    void parse_element_content(chptr_t& c)
    {
        while (true)
        {
            parse_element_value(c);

            if (*c != '<') throw parsing_exception(c);
            c++;

            if (*c == '/')
            {
                c++;
                parse_name(c);
                parse<'>'>(c);
                break;
            }
            else parse_element_name_end(c);
        }
    }

    template <typename chptr_t>
    void parse_element_name_end(chptr_t& c)
    {
        parse_name(c);
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