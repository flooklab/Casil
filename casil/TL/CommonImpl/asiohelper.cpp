/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
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

/// \cond INTERNAL

namespace casil::Layers::TL::CommonImpl::ASIOHelper
{

/*!
 * \brief Handler for socket transfer operations that does not fail when the socket gets cancelled.
 *
 * This is a handler prototype for asynchronous operations on Boost %ASIO's %TCP/%UDP sockets that transfer bytes and pass the number
 * of successfully transferred bytes to the handler (\p pNumBytes in this case). This handler treats both \e success and \e cancelled
 * error codes (\p pErrorCode equal to \c boost::system::errc::success or \c boost::system::errc::operation_canceled, respectively)
 * as successful outcome. This means that the value of \p pNumBytesPromise will be set to \p pNumBytes not only on success but also if
 * the socket gets cancelled before completion of the operation (i.e. retrieval of \e already transferred bytes from the promise/future is
 * possible). For any \e other error code the \e exception of \p pNumBytesPromise is set to the corresponding \c boost::system::system_error.
 *
 * Use this function with \c std::bind to bind \p pNumBytesPromise and pass the bound function as actual handler.
 *
 * \throws std::invalid_argument If \p pNumBytesPromise has no shared state or already stores a value/exception.
 *
 * \param pErrorCode Result/error code of the handled async operation.
 * \param pNumBytes Number of successfully transferred bytes.
 * \param pNumBytesPromise The promise to use to communicate the number of transferred bytes (or exception on unhandled error).
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

/// \endcond INTERNAL
