# Converts a binary file into a C++ source file containing a byte array.
# Invoked at build time: cmake -DINPUT=<file> -DOUTPUT=<file.cpp> -DSYMBOL=<name> -P embed.cmake
file(READ "${INPUT}" HEX_CONTENT HEX)
string(LENGTH "${HEX_CONTENT}" HEX_LEN)
math(EXPR BYTE_LEN "${HEX_LEN} / 2")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," BYTES "${HEX_CONTENT}")
file(WRITE "${OUTPUT}"
"// Generated from ${INPUT} - do not edit\n"
"#include <cstddef>\n"
"extern const unsigned char ${SYMBOL}[] = {${BYTES}};\n"
"extern const std::size_t ${SYMBOL}_size = ${BYTE_LEN};\n")
