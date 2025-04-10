#ifndef BASE64_H
#define BASE64_H

#include <string>

// Function to encode a string in Base64
std::string base64_encode(const std::string& input);

// Function to decode a Base64-encoded string
std::string base64_decode(const std::string& encoded_string);

#endif // BASE64_H
