#include <iostream>
#include "chimera/key.h"
#include "chimera/base.h"

int main (int argc, char** argv)
{
    if (argc == 2 || argc == 3) {
        Key k;
        key_makehash (NULL, &k, argv[1]);
        char keystr[KEY_SIZE];
        int base = 0;
        if (argc == 3) {
            base = atoi(argv[2]);
        }

        if (base == 4) {
            char tmpstr[KEY_SIZE];
            key_to_cstr(&k,tmpstr, KEY_SIZE);
            hex_to_base4 (tmpstr, keystr, KEY_SIZE);
        }
        else {
            key_to_cstr(&k,keystr, KEY_SIZE);
        }
        std::cout << keystr << std::endl;
    }
    else {
        std::cerr << "USAGE: " << argv[0] << " <STR_TO_HASH> [BASE]" << std::endl;
        return (-1);
    }
    return 0;
}
