/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#include <casil/logger.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(Logger_Tests)

BOOST_AUTO_TEST_CASE(Test1_logLevelThreshold)
{
    using casil::Logger;

    std::ostringstream logOutputStrm;

    Logger::addOutput(logOutputStrm);

    Logger::setLogLevel(Logger::LogLevel::Verbose);

    Logger::log("HelloWorld0-Level-None", Logger::LogLevel::None);
    Logger::log("HelloWorld1-Level-Critical", Logger::LogLevel::Critical);
    Logger::log("HelloWorld2-Level-Error", Logger::LogLevel::Error);
    Logger::log("HelloWorld3-Level-Warning", Logger::LogLevel::Warning);
    Logger::log("HelloWorld4-Level-Success", Logger::LogLevel::Success);
    Logger::log("HelloWorld5-Level-Info", Logger::LogLevel::Info);
    Logger::log("HelloWorld6-Level-More", Logger::LogLevel::More);
    Logger::log("HelloWorld7-Level-Verbose", Logger::LogLevel::Verbose);
    Logger::log("HelloWorld8-Level-Debug", Logger::LogLevel::Debug);
    Logger::log("HelloWorld9-Level-DebugDebug", Logger::LogLevel::DebugDebug);

    Logger::setLogLevel(Logger::LogLevel::DebugDebug);
    Logger::log("HelloWorld8.1-Level-Debug", Logger::LogLevel::Debug);
    Logger::log("HelloWorld9.1-Level-DebugDebug", Logger::LogLevel::DebugDebug);
    Logger::setLogLevel(Logger::LogLevel::Verbose);

    Logger::removeOutput(logOutputStrm);

    Logger::log("HelloWorld10", Logger::LogLevel::Verbose);

    std::string testStr = logOutputStrm.str();

    BOOST_CHECK(testStr.find("HelloWorld0-Level-None") == testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld1-Level-Critical") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld2-Level-Error") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld3-Level-Warning") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld4-Level-Success") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld5-Level-Info") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld6-Level-More") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld7-Level-Verbose") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld8-Level-Debug") == testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld8.1-Level-Debug") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld9-Level-DebugDebug") == testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld9.1-Level-DebugDebug") != testStr.npos);
    BOOST_CHECK(testStr.find("HelloWorld10") == testStr.npos);
}

BOOST_AUTO_TEST_CASE(Test2_logFile)
{
    using casil::Logger;

    std::filesystem::path tmpPath = std::filesystem::temp_directory_path();

    std::string logFileName = tmpPath / ("tmp" + std::to_string(std::rand()) + ".log");

    BOOST_REQUIRE(!std::filesystem::exists(logFileName));

    Logger::addLogFile(logFileName);

    Logger::setLogLevel(Logger::LogLevel::Info);

    Logger::logInfo("This is a test message.");

    Logger::removeLogFile(logFileName);

    Logger::logInfo("This is the second test message.");

    std::ifstream logFile;

    logFile.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    logFile.open(logFileName);

    std::string logStr;

    try
    {
        for (std::string line; std::getline(logFile, line, '\n');)
            logStr.append(line).append("\n");
    }
    catch (const std::ios_base::failure&)
    {
        if (!logFile.eof())
            throw;
    }

    logFile.close();

    std::filesystem::remove(logFileName);

    BOOST_CHECK(logStr.find("This is a test message.") != logStr.npos);
    BOOST_CHECK(logStr.find("This is the second test message.") == logStr.npos);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
