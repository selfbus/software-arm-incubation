/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

// The sblib expects the setup() of an application; the tests don't start the application.
class BcuBase;
BcuBase* setup()
{
    return nullptr;
}
