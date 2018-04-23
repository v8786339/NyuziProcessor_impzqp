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

#pragma once

// Memory mapped peripheral registers

static volatile unsigned int * const REGISTERS = (volatile unsigned int*) 0xffff0000;

enum register_index
{
    REG_RED_LED             = 0x0000 / 4,
    REG_GREEN_LED           = 0x0004 / 4,
    REG_HEX0                = 0x0008 / 4,
    REG_HEX1                = 0x000c / 4,
    REG_HEX2                = 0x0010 / 4,
    REG_HEX3                = 0x0014 / 4,
    REG_UART_RX             = 0x2000 / 4,
    REG_UART_TX             = 0x2004 / 4,
    REG_UART_STATUS         = 0x2008 / 4,
    REG_UART_CTRL           = 0x200c / 4,
    REG_KB_STATUS           = 0x3000 / 4,
    REG_KB_SCANCODE         = 0x3004 / 4,
    REG_SD_SPI_WRITE        = 0x4000 / 4,
    REG_SD_SPI_READ         = 0x4004 / 4,
    REG_SD_SPI_STATUS       = 0x4008 / 4,
    REG_SD_SPI_CONTROL      = 0x400c / 4,
    REG_SD_SPI_CLOCK_DIVIDE = 0x4010 / 4,
    REG_THREAD_RESUME       = 0x5000 / 4,
    REG_THREAD_HALT         = 0x5004 / 4,
    REG_VGA_ENABLE          = 0x1000 / 4,
    REG_VGA_MICROCODE       = 0x1004 / 4,
    REG_VGA_BASE            = 0x1008 / 4,
    REG_VGA_LENGTH          = 0x100c / 4,
    REG_PERF0_SEL           = 0x6000 / 4,
    REG_PERF1_SEL           = 0x6004 / 4,
    REG_PERF2_SEL           = 0x6008 / 4,
    REG_PERF3_SEL           = 0x600c / 4,
    REG_PERF0_VAL           = 0x6010 / 4,
    REG_PERF1_VAL           = 0x6014 / 4,
    REG_PERF2_VAL           = 0x6018 / 4,
    REG_PERF3_VAL           = 0x601c / 4
};

