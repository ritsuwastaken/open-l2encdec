#ifndef OPTIONS_H
#define OPTIONS_H

#include <l2encdec.h>
#include <string>

struct Options
{
    l2encdec::Params params;
    std::string input_file;
    std::string output_filename;
    bool verify;
    l2encdec::Type algorithm;
    bool is_encode;
};

namespace options
{
    Options parse(int argc, char *argv[]);
} // namespace options

#endif // OPTIONS_H
