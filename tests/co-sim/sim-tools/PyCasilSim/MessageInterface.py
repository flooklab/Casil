##################################################################################################
#
#  Copyright (C) 2026 M. Frohne
#
#  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
#
#  Casil is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Affero General Public License as published
#  by the Free Software Foundation, either version 3 of the License,
#  or (at your option) any later version.
#
#  Casil is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with Casil. If not, see <https://www.gnu.org/licenses/>.
#
##################################################################################################
#
#  This file has been directly adapted from the corresponding code of
#  the basil software, which itself is covered by the following license:
#
#      Copyright (C) 2011–2026 SiLab, Institute of Physics, University of Bonn
#
#      All rights reserved.
#
#      Redistribution and use in source and binary forms, with or without
#      modification, are permitted provided that the following conditions are
#      met:
#
#       *  Redistributions of source code must retain the above copyright notice,
#          this list of conditions and the following disclaimer.
#
#       *  Redistributions in binary form must reproduce the above copyright
#          notice, this list of conditions and the following disclaimer in the
#          documentation and/or other materials provided with the distribution.
#
#       *  Neither the name of the copyright holder nor the names of its
#          contributors may be used to endorse or promote products derived from
#          this software without specific prior written permission.
#
#      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
#      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##################################################################################################
#
#  Initial version by Chris Higgs <chris.higgs@potentialventures.com>
#
##################################################################################################

import PyCasil
SiSim = PyCasil.Layers.TL.SiSim

import socket

class MessageInterface:
    """Simulation-side counterpart of the software-side SiSim interface."""

    def __init__(self, sock):
        """Use the passed socket to receive/send messages from/to."""
        self.sock = sock

    def sendResponse(self, data):
        """Send a response to a read request to the socket."""
        self.sock.sendall(bytes(SiSim.createReadResponse(data)))

    def receiveRequest(self):
        """Receive the next request from the socket."""
        return self._receiveMessage(SiSim.parseMessageType(self.sock.recv(1)[0]), SiSim.parseMessageSize(list(self.sock.recv(4))))

    def tryReceiveRequest(self):
        """Receive the next request from the socket, return None immediately if nothing is waiting."""
        try:
            msgHeader = list(self.sock.recv(5, socket.MSG_DONTWAIT))
        except BlockingIOError:
            return (None, None, None)

        if len(msgHeader) == 0:
            raise EOFError("Socket closed.")

        return self._receiveMessage(SiSim.parseMessageType(msgHeader[0]), SiSim.parseMessageSize(msgHeader[1:5]))

    def _receiveMessage(self, msgType, msgSize):
        """Receive the message payload from the socket and parse it as read or write request."""
        if msgType == SiSim.MessageType.WriteRequest:
            return (msgType, ) + SiSim.parseWriteRequest(list(self.sock.recv(msgSize)))
        elif msgType == SiSim.MessageType.ReadRequest:
            return (msgType, ) + SiSim.parseReadRequest(list(self.sock.recv(msgSize)))
        else:
            raise RuntimeError("Invalid message type: %s" % str(msgType))
