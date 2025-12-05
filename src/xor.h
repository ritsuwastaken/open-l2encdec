#ifndef XOR_H
#define XOR_H

#include <functional>
#include <string>
#include <vector>

namespace XOR
{
using KeyGenerator = std::function<int(int)>;

size_t encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key);
size_t encrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator);
size_t decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int xor_key);
size_t decrypt(const std::vector<unsigned char> &input, std::vector<unsigned char> &output, int start_index, KeyGenerator key_generator);
int get_key_by_index(int index);
int get_key_by_filename(std::string filename);
} // namespace XOR

#endif // XOR_H
