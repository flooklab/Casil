/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024 M. Frohne
//
//  Casil is free software: you can redistribute it and/or modify it
//  under the terms of the GNU Affero General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Casil is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty
//  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with Casil. If not, see <https://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef CASILTESTS_DATADIRFIXTURE_H
#define CASILTESTS_DATADIRFIXTURE_H

#include <boost/test/unit_test.hpp>

#include <string>
#include <type_traits>

struct DataDirFixture
{
    DataDirFixture() :
#ifdef CASIL_TEST_DATA_DIR
#define CASIL_XSTR(S) CASIL_STR(S)
#define CASIL_STR(S) #S
        dataPath(CASIL_XSTR(CASIL_TEST_DATA_DIR))
#else
        dataPath(extractDataPath())
#endif
    {
    }
    ~DataDirFixture() = default;
    //
    const std::string dataPath;

private:
    static constexpr const char dataPathArgvStart[] = "--test-data-dir=";
    static constexpr auto dataPathArgvStartLen = std::extent_v<decltype(dataPathArgvStart)> - 1;
    //
    static std::string extractDataPath()
    {
        if (boost::unit_test::framework::master_test_suite().argc == 2)
        {
            std::string argv1(boost::unit_test::framework::master_test_suite().argv[1]);

            if (argv1.length() > dataPathArgvStartLen && argv1.substr(0, dataPathArgvStartLen) == std::string(dataPathArgvStart))
                return argv1.substr(dataPathArgvStartLen);
        }

        return "";
    }
};

#endif // CASILTESTS_DATADIRFIXTURE_H
