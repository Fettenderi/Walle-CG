#ifndef I_PICKABLE_H
#define I_PICKABLE_H

#include "character.h"

class IPickable {
    public:
        virtual bool canBePicked(Character picker) {
            return true;
        }
};

#endif