#pragma once

namespace intxml
{
    class line_counter
    {
        int lin;
        int col;
        int last_char;

    public:
        line_counter() : lin(1), col(0), last_char(0) {}

        template <typename char_t>
        void update(char_t c)
        {
            if (c == '\n')
            {
                if (last_char != '\r')
                {
                    col = 0;
                    lin++;
                }
            }
            else if (c == '\r')
            {
                col = 0;
                lin++;
            }
            else
            {
                col++;
            }

            last_char = c;
        }

        int line() { return lin; }
        int column() { return col; }
    };
}