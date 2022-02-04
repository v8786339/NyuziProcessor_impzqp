//
// Copyright 2011-2015 Jeff Bush
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

`include "defines.svh"

import defines::*;

//
// Top level block for processor. Contains all cores and L2 cache, connects
// to AXI system bus.
//

module nyuzi
    #(parameter RESET_PC = 0,
    parameter NUM_INTERRUPTS = 16,
     //M_IO_AXI Parameters
    parameter  C_M_IO_TARGET_SLAVE_BASE_ADDR    = 32'h00000000,
    parameter integer C_M_IO_AXI_ADDR_WIDTH    = 32,
    parameter integer C_M_IO_AXI_DATA_WIDTH    = 32)

    (input                          clk,
    input                           reset,
    axi4_interface.master           axi_bus,
    //io_bus_interface.master         io_bus,
    jtag_interface.target           jtag,
    input [NUM_INTERRUPTS - 1:0]    interrupt_req,
     //M_IO_AXI
    output wire [C_M_IO_AXI_ADDR_WIDTH-1 : 0] m_io_axi_awaddr,
    output wire [2 : 0] m_io_axi_awprot,
    output wire  m_io_axi_awvalid,
    input wire  m_io_axi_awready,
    output wire [C_M_IO_AXI_DATA_WIDTH-1 : 0] m_io_axi_wdata,
    output wire [C_M_IO_AXI_DATA_WIDTH/8-1 : 0] m_io_axi_wstrb,
    output wire  m_io_axi_wvalid,
    input wire  m_io_axi_wready,
    input wire [1 : 0] m_io_axi_bresp,
    input wire  m_io_axi_bvalid,
    output wire  m_io_axi_bready,
    output wire [C_M_IO_AXI_ADDR_WIDTH-1 : 0] m_io_axi_araddr,
    output wire [2 : 0] m_io_axi_arprot,
    output wire  m_io_axi_arvalid,
    input wire  m_io_axi_arready,
    input wire [C_M_IO_AXI_DATA_WIDTH-1 : 0] m_io_axi_rdata,
    input wire [1 : 0] m_io_axi_rresp,
    input wire  m_io_axi_rvalid,
    output wire  m_io_axi_rready);

    l2req_packet_t l2i_request[`NUM_CORES];
    logic[`NUM_CORES - 1:0] l2i_request_valid;
    ioreq_packet_t ior_request[`NUM_CORES];
    logic[`NUM_CORES - 1:0] ior_request_valid;
    logic[TOTAL_THREADS - 1:0] thread_en;
    scalar_t cr_data_to_host[`NUM_CORES];
    scalar_t data_to_host;
    logic[`NUM_CORES - 1:0] core_injected_complete;
    logic[`NUM_CORES - 1:0] core_injected_rollback;
    logic[`NUM_CORES - 1:0][TOTAL_THREADS - 1:0] core_suspend_thread;
    logic[`NUM_CORES - 1:0][TOTAL_THREADS - 1:0] core_resume_thread;
    logic[TOTAL_THREADS - 1:0] thread_suspend_mask;
    logic[TOTAL_THREADS - 1:0] thread_resume_mask;

    /*AUTOLOGIC*/
    // Beginning of automatic wires (for undeclared instantiated-module outputs)
    logic               ii_ready [`NUM_CORES];  // From io_interconnect of io_interconnect.v
    iorsp_packet_t      ii_response;            // From io_interconnect of io_interconnect.v
    logic               ii_response_valid;      // From io_interconnect of io_interconnect.v
    logic               l2_ready [`NUM_CORES];  // From l2_cache of l2_cache.v
    l2rsp_packet_t      l2_response;            // From l2_cache of l2_cache.v
    logic               l2_response_valid;      // From l2_cache of l2_cache.v
    core_id_t           ocd_core;               // From on_chip_debugger of on_chip_debugger.v
    scalar_t            ocd_data_from_host;     // From on_chip_debugger of on_chip_debugger.v
    logic               ocd_data_update;        // From on_chip_debugger of on_chip_debugger.v
    logic               ocd_halt;               // From on_chip_debugger of on_chip_debugger.v
    logic               ocd_inject_en;          // From on_chip_debugger of on_chip_debugger.v
    scalar_t            ocd_inject_inst;        // From on_chip_debugger of on_chip_debugger.v
    local_thread_idx_t  ocd_thread;             // From on_chip_debugger of on_chip_debugger.v
    // End of automatics

    initial
    begin
        // Check config (see config.svh for rules)
        assert(`NUM_CORES >= 1 && `NUM_CORES <= (1 << CORE_ID_WIDTH));
        assert(`L1D_WAYS >= `THREADS_PER_CORE);
        assert(`L1I_WAYS >= `THREADS_PER_CORE);
    end


    //***THREAD ENABLE***
    //INTERMEDIATE SIGNALS
    logic m_io_axi_awready_io_interconnect;
    logic m_io_axi_wready_io_interconnect;
    logic[1 : 0] m_io_axi_bresp_io_interconnect;
    logic m_io_axi_bvalid_io_interconnect;
    logic m_io_axi_awvalid_io_interconnect;
    logic m_io_axi_wvalid_io_interconnect;
    logic m_io_axi_awvalid_out;
    logic m_io_axi_wvalid_out;
    parameter [0:0] IDLE = 1'b0, CONFIRM = 1'b1;
    logic[0:0] axi_thread_ctrl_state;
    
    always_ff @(posedge clk)
    begin
        if(reset)
        begin
            axi_thread_ctrl_state <= IDLE;
        end
        else
        begin
            case(axi_thread_ctrl_state)
                IDLE:
                begin
                    //If write in 0x5XXX Address Range
                    if(m_io_axi_awvalid_io_interconnect & m_io_axi_wvalid_io_interconnect & m_io_axi_awaddr[14] & m_io_axi_awaddr[12])
                    begin
                        axi_thread_ctrl_state <= CONFIRM;
                    end
                end
                CONFIRM:
                begin
                    if(m_io_axi_bready)
                    begin
                        axi_thread_ctrl_state <= IDLE;
                    end
                end
                default:
                begin
                    axi_thread_ctrl_state <= IDLE;
                end
            endcase
        end
    end
    
    always_comb
    begin
        //Route Signals from ports
        m_io_axi_bvalid_io_interconnect     = m_io_axi_bvalid;
        m_io_axi_awready_io_interconnect    = m_io_axi_awready;
        m_io_axi_wready_io_interconnect     = m_io_axi_wready;
        m_io_axi_bresp_io_interconnect      = m_io_axi_bresp;
        //Route Signals to Ports
        m_io_axi_awvalid_out    = m_io_axi_awvalid_io_interconnect;
        m_io_axi_wvalid_out     = m_io_axi_wvalid_io_interconnect;
        case(axi_thread_ctrl_state)
            IDLE:
            begin
                //Drive Signals locally
                if(m_io_axi_awvalid_io_interconnect & m_io_axi_awaddr[14] & m_io_axi_awaddr[12])
                begin
                    //DEFAULT
                    //To io_interconnect
                    m_io_axi_bvalid_io_interconnect     = 0;
                    m_io_axi_awready_io_interconnect    = 0;
                    m_io_axi_wready_io_interconnect     = 0;
                    m_io_axi_bresp_io_interconnect      = 0;
                    //To Ports
                        //The valid Signals are cut off the bus, in order to prevent an
                        //external AXI Slave responding to the internally handled transaction
                    m_io_axi_awvalid_out    = 0;
                    m_io_axi_wvalid_out     = 0;
                    
                    if(m_io_axi_wvalid_io_interconnect)
                    begin
                        m_io_axi_awready_io_interconnect    = 1;
                        m_io_axi_wready_io_interconnect     = 1;
                    end
                end
            end
            CONFIRM:
            begin
                //DEFAULT
                //To io_interconnect
                m_io_axi_bvalid_io_interconnect     = 1;
                m_io_axi_awready_io_interconnect    = 0;
                m_io_axi_wready_io_interconnect     = 0;
                m_io_axi_bresp_io_interconnect      = 0;
                //To Ports
                    //The valid Signals are cut off the bus, in order to prevent an
                    //external AXI Slave responding to the internally handled transaction
                m_io_axi_awvalid_out    = 0;
                m_io_axi_wvalid_out     = 0;
            end
        endcase
    end
    
    assign m_io_axi_awvalid = m_io_axi_awvalid_out;
    assign m_io_axi_wvalid  = m_io_axi_wvalid_out;

    // Thread enable
    always @*
    begin
        thread_suspend_mask = '0;
        thread_resume_mask = '0;
        for (int i = 0; i < `NUM_CORES; i++)
        begin
            thread_suspend_mask |= core_suspend_thread[i];
            thread_resume_mask |= core_resume_thread[i];
        end
    end

    always_ff @(posedge clk, posedge reset)
    begin
        if (reset)
            thread_en <= 1;
        else
            thread_en <= (thread_en | thread_resume_mask) & ~thread_suspend_mask;
    end

    l2_cache l2_cache(
        .l2_perf_events(),
        .*);

     io_interconnect 
        # (
            .C_M_IO_TARGET_SLAVE_BASE_ADDR(C_M_IO_TARGET_SLAVE_BASE_ADDR),
            .C_M_IO_AXI_ADDR_WIDTH(C_M_IO_AXI_ADDR_WIDTH),
            .C_M_IO_AXI_DATA_WIDTH(C_M_IO_AXI_DATA_WIDTH)
        )
        io_interconnect(
//        .io_bus(interconnect_io_bus),
        .m_io_axi_awready(m_io_axi_awready_io_interconnect),
        .m_io_axi_wready(m_io_axi_wready_io_interconnect),
        .m_io_axi_bresp(m_io_axi_bresp_io_interconnect),
        .m_io_axi_bvalid(m_io_axi_bvalid_io_interconnect),
        .m_io_axi_awvalid(m_io_axi_awvalid_io_interconnect),
        .m_io_axi_wvalid(m_io_axi_wvalid_io_interconnect),
        .*);

    on_chip_debugger on_chip_debugger(
        .jtag(jtag),
        .injected_complete(|core_injected_complete),
        .injected_rollback(|core_injected_rollback),
        .*);

    generate
        if (`NUM_CORES > 1)
            assign data_to_host = cr_data_to_host[CORE_ID_WIDTH'(ocd_core)];
        else
            assign data_to_host = cr_data_to_host[0];
    endgenerate

    genvar core_idx;
    generate
        for (core_idx = 0; core_idx < `NUM_CORES; core_idx++)
        begin : core_gen
            core #(
                .CORE_ID(core_id_t'(core_idx)),
                .NUM_INTERRUPTS(NUM_INTERRUPTS),
                .RESET_PC(RESET_PC)
            ) core(
                .l2i_request_valid(l2i_request_valid[core_idx]),
                .l2i_request(l2i_request[core_idx]),
                .l2_ready(l2_ready[core_idx]),
                .thread_en(thread_en[core_idx * `THREADS_PER_CORE+:`THREADS_PER_CORE]),
                .ior_request_valid(ior_request_valid[core_idx]),
                .ior_request(ior_request[core_idx]),
                .ii_ready(ii_ready[core_idx]),
                .ii_response(ii_response),
                .cr_data_to_host(cr_data_to_host[core_idx]),
                .injected_complete(core_injected_complete[core_idx]),
                .injected_rollback(core_injected_rollback[core_idx]),
                .cr_suspend_thread(core_suspend_thread[core_idx]),
                .cr_resume_thread(core_resume_thread[core_idx]),
                .*);
        end
    endgenerate
endmodule
