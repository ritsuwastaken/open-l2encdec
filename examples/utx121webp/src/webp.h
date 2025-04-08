#ifndef WEBP_H
#define WEBP_H

#include <vector>

namespace webp
{
    void from_dxt1(const std::vector<unsigned char> &compressed, int width, int height, std::vector<unsigned char> &output);
    void from_dxt3(const std::vector<unsigned char> &compressed, int width, int height, std::vector<unsigned char> &output);
}

#endif // WEBP_H
