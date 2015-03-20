#pragma once

#include "intxml.h"

// This file is an attempt to create an interface to a document that is 
// slightly higher-level than intxml.h.  It defines a series of classes that 
// represent states of an XML parsing process.  Each state provides routines
// that allow the caller to proceeed to the next state.  Associated with each 
// state is a pointer into the document memory.

namespace intxml { namespace parser
{
    class parser_exception : public std::exception
    {
    public:
        template <typename chptr_t>
        parser_exception(const chptr_t& p) {}
    };

    // Declaration of all parser state objects
    template <typename chptr_t> class document;
    template <typename chptr_t> class element;
    template <typename chptr_t> class attribute;
    template <typename chptr_t> class attribute_value;
    template <typename chptr_t> class content;

    // Text content of an element.  The content length may be 0, e.g., 
    // between elements "<a/><b/>".
    template <typename chptr_t>
    class content
    {
        chptr_t p;

    public:
        content(chptr_t ptr) : p(ptr) {}

        // Parses the content up to the next element or close tag and 
        // returns the element (possibly representing the close tag).
        element<chptr_t> sibling()
        {
            chptr_t pnew(p);
            parse_element_text(pnew);
            return element<chptr_t>(pnew);
        }

        template <typename handler>
        element<chptr_t> sibling(handler h)
        {
            text_ptr<chptr_t> tp(p);
            h(tp);
            while (*tp) tp++;
            return element<chptr_t>(tp.ptr());
        }
    };

    // Just before the value part of an attribute
    template <typename chptr_t>
    class attribute_value
    {
        chptr_t p;

    public:
        attribute_value(chptr_t ptr) : p(ptr) {}

        // Parse the value and return the next attribute (possibly 
        // representing the end of the attribute list).
        attribute<chptr_t> value()
        {
            chptr_t pnew(p);
            parse_attribute_value(pnew);
            parse_whitespace(pnew);
            return attribute<chptr_t>(pnew);
        }

        template <typename handler>
        attribute<chptr_t> value(handler h)
        {
            attribute_value_ptr<chptr_t> vp(p);
            h(vp);
            while (*vp) vp++;
            parse_whitespace(vp.ptr());
            return attribute<chptr_t>(vp.ptr());
        }
    };

    // Just before the name part of an attribute or the '/>' or '>' at the 
    // end of an element open tag.
    template <typename chptr_t>
    class attribute
    {
        chptr_t p;

    public:
        enum next_types { attribute_name, child_content, sibling_content };

        attribute(chptr_t ptr) : p(ptr) {}

        // Returns what follows in the document, either an attribute, child 
        // content, or sibling content (if open tag ends with "/>").
        next_types next()
        {
            switch (*p)
            {
            case '/': return sibling_content;
            case '>': return child_content;
            default: return attribute_name;
            }
        }

        // Parse the attribute name and return the value.  Throws an 
        // exception if there are no more attributes (has_more() is false).
        attribute_value<chptr_t> name()
        {
            chptr_t pnew(p);
            parse_name(pnew);
            parse_whitespace(pnew);
            parse<'='>(pnew);
            parse_whitespace(pnew);
            return attribute_value<chptr_t>(pnew);
        }

        // Same as above, but calls the supplied handler to process the 
        // attribute name
        template <typename handler>
        attribute_value<chptr_t> name(handler h)
        {
            name_ptr<chptr_t> np(p);
            h(np);
            while (*np) np++;
            chptr_t pnew(np.ptr());
            parse_whitespace(pnew);
            parse<'='>(pnew);
            parse_whitespace(pnew);
            return attribute_value<chptr_t>(pnew);
        }

        // Returns an object pointing to the element content.  Throws an 
        // exception if there is no child content (open tag ends with "/>").
        content<chptr_t> child()
        {
            chptr_t pnew(p);
            parse_attributes(pnew);
            if (!parse_start_tag_end(pnew)) throw parser_exception(p);
            return content<chptr_t>(pnew);
        }

        // Returns an object pointing to the content following this element.  
        // Note that this skips over any remaining attributes and child 
        // elements.
        content<chptr_t> sibling()
        {
            chptr_t pnew(p);
            parse_element_attribute_end(pnew);
            return content<chptr_t>(pnew);
        }
    };

    // Just before the name portion in an element open tag, or the '/' in an 
    // end tag.
    template <typename chptr_t>
    class element
    {
        chptr_t p;

    public:
        element(chptr_t ptr) : p(ptr) {}

        enum next_types { element_name, close_tag, end_of_doc };

        // Returns true only if there are elements remaining in the content 
        // of the parent element.
        next_types next()
        {
            switch (*p)
            {
            case '/': return close_tag;
            case 0: return end_of_doc;
            default: return element_name;
            }
        }

        // Parse the name and return the first attribute
        attribute<chptr_t> name()
        {
            chptr_t pnew(p);
            parse_name(pnew);
            parse_whitespace(pnew);
            return attribute<chptr_t>(pnew);
        }

        // Same as above, but uses the supplied handler to process the name
        template <typename handler>
        attribute<chptr_t> name(handler h)
        {
            name_ptr<chptr_t> np(p);
            h(np);
            chptr_t pnew(np.ptr());
            while (*np) np++;
            parse_whitespace(pnew);
            return attribute<chptr_t>(pnew);
        }

        // Parses the close tag and returns the following content
        content<chptr_t> close()
        {
            chptr_t pnew(p);
            parse<'/'>(pnew);
            parse_name(pnew);
            parse<'>'>(pnew);
            return content<chptr_t>(pnew);
        }
    };

    // Initial state, before anything has been processed.
    template <typename chptr_t>
    class document
    {
        chptr_t p;

    public:
        document(chptr_t ptr) : p(ptr) {}

        // Parses the prolog and returns the root element.
        element<chptr_t> root()
        {
            chptr_t pnew(p);
            parse_prolog(pnew);
            return element<chptr_t>(pnew);
        }
    };

}}