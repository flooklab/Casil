/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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
//
//  This file has been directly adapted from the corresponding code of
//  the basil software, which itself is covered by the following license:
//
//      Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn
//
//      All rights reserved.
//
//      Redistribution and use in source and binary forms, with or without
//      modification, are permitted provided that the following conditions are
//      met:
//
//       *  Redistributions of source code must retain the above copyright notice,
//          this list of conditions and the following disclaimer.
//
//       *  Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//
//       *  Neither the name of the copyright holder nor the names of its
//          contributors may be used to endorse or promote products derived from
//          this software without specific prior written permission.
//
//      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

`timescale 1ps / 1ps

`include "utils/bus_to_ip.v"

`include "i2c/i2c.v"
`include "i2c/i2c_core.v"

`include "utils/cdc_pulse_sync.v"
`include "utils/clock_divider.v"

`include "utils/ODDR_sim.v"
`include "utils/IDDR_sim.v"

module i2c_slave_model (
    input wire SCL,
    inout wire SDA
);

parameter ADDRESS = 7'b1001001;

reg START;

//initial START = 1;

always @(negedge SDA or negedge SCL)
    if(~SCL)
        START <= 0;
    else
        START <= 1;

reg [7:0] REC_ADDRESS;

localparam STATE_IDLE  = 0, STATE_START = 1, STATE_ADDR = 2, STATE_AACK = 4, STATE_DATA_W = 5, STATE_DATA_R = 6, STATE_DACK_W = 7, STATE_DACK_R = 8, STATE_DACK_LAST = 9, STATE_STOP = 10;

reg [3:0] state, next_state;

reg [2:0] bit_count;
reg [15:0] byte_count;

initial state = STATE_IDLE;

always @(posedge SCL or posedge START) begin
    if (START)
        state <= STATE_ADDR;
    else
        state <= next_state;
end

//rec stop
always @(*) begin
    next_state = state;
    case(state)
        STATE_IDLE:
            next_state = state;
        STATE_ADDR:
            if(bit_count==7)
                next_state = STATE_AACK;
        STATE_AACK:
            if(REC_ADDRESS[7:1] == ADDRESS) begin
                if(REC_ADDRESS[0])
                    next_state = STATE_DATA_R;
                else
                    next_state = STATE_DATA_W;
            end
            else
                next_state = STATE_IDLE;
        STATE_DATA_R:
            if(bit_count==7)
                next_state = STATE_DACK_R;
        STATE_DATA_W:
            if(bit_count==7)
                next_state = STATE_DACK_W;
        STATE_DACK_W:
            next_state = STATE_DATA_W;
        STATE_DACK_R:
            if(SDA==0)
                next_state = STATE_DATA_R;
            else
                next_state = STATE_IDLE;
    endcase
end

always @(posedge SCL or posedge START) begin
    if(START)
        bit_count <= 0;
    else if (state == STATE_AACK | state == STATE_DACK_R | state == STATE_DACK_W )
        bit_count <= 0;
    else
        bit_count <= bit_count + 1;
end

always @(posedge SCL or posedge START) begin
    if (START)
        byte_count <= 0;
    else if(next_state == STATE_DACK_W | next_state == STATE_DACK_R)
        byte_count <= byte_count + 1;
end

always @(posedge SCL)
    if(state == STATE_ADDR)
        REC_ADDRESS[7-bit_count] = SDA;

reg [7:0] BYTE_DATA_IN;
always @(posedge SCL)
    if(state == STATE_DATA_W)
        BYTE_DATA_IN[7-bit_count] = SDA;

reg [7:0] mem_addr;
always @(posedge SCL) begin
    if(byte_count == 0 & next_state == STATE_DACK_W )
        mem_addr <= BYTE_DATA_IN;
    else if(next_state == STATE_DACK_R | next_state == STATE_DACK_W)
        mem_addr <= mem_addr + 1;
end

reg [7:0] mem [255:0];
wire MEM_WE;
assign MEM_WE = next_state == STATE_DACK_W & byte_count != 0;

always @(posedge SCL or posedge START)
    if(MEM_WE) begin
        mem[mem_addr] <= BYTE_DATA_IN;
end

wire [7:0] BYTE_DATA_OUT;
assign BYTE_DATA_OUT = mem[mem_addr];

wire SDA_PRE;
assign SDA_PRE = ((state == STATE_AACK & REC_ADDRESS[7:1] == ADDRESS) | state == STATE_DACK_W) ? 1'b0 : state == STATE_DATA_R ? BYTE_DATA_OUT[7-bit_count] : 1;
reg SDAR;
initial SDAR = 1;
always @(negedge SCL)
    SDAR <= SDA_PRE;

assign SDA = SDAR ? 1'bz : 1'b0;

endmodule

module tb (
    input wire          BUS_CLK,
    input wire          BUS_RST,
    input wire  [31:0]  BUS_ADD,
    inout wire  [31:0]  BUS_DATA,
    input wire          BUS_RD,
    input wire          BUS_WR,
    output wire         BUS_BYTE_ACCESS
);

// MODULE ADREESSES //
localparam I2C_BASEADDR = 32'h1000;
localparam I2C_HIGHADDR = 32'h2000-1;

localparam ABUSWIDTH = 32;
assign BUS_BYTE_ACCESS = BUS_ADD < 32'h8000_0000 ? 1'b1 : 1'b0;

wire I2C_CLK;

clock_divider #(
    .DIVISOR(4)
) i_clock_divisor_spi (
    .CLK(BUS_CLK),
    .RESET(1'b0),
    .CE(),
    .CLOCK(I2C_CLK)
);

wire SDA, SCL;

pullup  isda (SDA);
pullup  iscl (SCL);

i2c #(
    .BASEADDR(I2C_BASEADDR),
    .HIGHADDR(I2C_HIGHADDR),
    .ABUSWIDTH(ABUSWIDTH),
    .MEM_BYTES(32)
) i_i2c (
    .BUS_CLK(BUS_CLK),
    .BUS_RST(BUS_RST),
    .BUS_ADD(BUS_ADD),
    .BUS_DATA(BUS_DATA[7:0]),
    .BUS_RD(BUS_RD),
    .BUS_WR(BUS_WR),

    .I2C_CLK(I2C_CLK),
    .I2C_SDA(SDA),
    .I2C_SCL(SCL)
);

i2c_slave_model ii2c_slave_model (.SDA(SDA), .SCL(SCL));

initial begin
    $dumpfile("i2c.vcd");
    $dumpvars(0);
end

endmodule
