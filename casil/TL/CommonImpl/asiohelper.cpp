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

#include <casil/TL/CommonImpl/asiohelper.h>

#include <boost/system/system_error.hpp>

namespace casil::TL::CommonImpl::ASIOHelper
{

/*!
 * \brief Handler for socket transfer operations that does not fail when the socket gets cancelled.
 *
 * \todo Detailed doc
 *
 * \param pErrorCode
 * \param pNumBytes
 * \param pNumBytesPromise
 */
void readWriteHandler(const boost::system::error_code& pErrorCode, const std::size_t pNumBytes, std::promise<std::size_t>& pNumBytesPromise)
{
    if (pErrorCode.value() != boost::system::errc::success && pErrorCode.value() != boost::system::errc::operation_canceled)
    {
        try
        {
            pNumBytesPromise.set_exception(std::make_exception_ptr(boost::system::system_error(pErrorCode)));
        }
        catch (const std::future_error&)
        {
            throw std::invalid_argument("Invalid promise.");
        }
    }
    else
    {
        try
        {
            pNumBytesPromise.set_value(pNumBytes);
        }
        catch (const std::future_error&)
        {
            throw std::invalid_argument("Invalid promise.");
        }
    }
}

} // namespace casil::TL::CommonImpl::ASIOHelper
