#pragma once

namespace nya_memory
{

template<class t>
t &get_invalid_object()
{
    static t invalid_object;
    invalid_object.~t();
    new (&invalid_object) t();
    return invalid_object;
}

}
