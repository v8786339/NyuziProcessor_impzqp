`timescale 1ns / 1ps


    module io_bus_bridge#(
        parameter integer NUM_PERIPHERALS = 1
    )(
        io_bus_interface.master peripherals[NUM_PERIPHERALS - 1 : 0],
        io_bus_interface.slave io_bus
    );

    genvar i;
    generate
        for (i = 0; i < NUM_PERIPHERALS; i++)
        begin : io_gen
            assign peripherals[i].write_en = io_bus.write_en;
            assign peripherals[i].read_en = io_bus.read_en;
            assign peripherals[i].address = io_bus.address;
            assign peripherals[i].write_data = io_bus.write_data;
        end
    endgenerate
    
    
    always_comb begin
        case(io_bus.address)
            'h0: io_bus.read_data = peripherals[0].read_data;
        default:
            io_bus.read_data = 'h0;
        endcase
        
    end

endmodule
