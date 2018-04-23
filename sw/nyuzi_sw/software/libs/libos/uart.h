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

#ifdef __cplusplus
extern "C" {
#endif

#define UART_RX_READY 		(1 << 0)
#define UART_RX_FIFO_FULL	(1 << 1)
#define UART_TX_FIFO_EMPTY	(1 << 2)
#define UART_TX_FIFO_FULL 	(1 << 3)
#define UART_INTR_ENABLED	(1 << 4)
#define UART_OVERRUN 		(1 << 5)
#define UART_FRAME_ERR 		(1 << 6)
#define UART_PARITY_ERR		(1 << 7)

#define UART_CTRL_RST_TX_FIFO	(1 << 0)
#define UART_CTRL_RST_RX_FIFO	(1 << 1)
#define UART_CTRL_ENABLE_INTR	(1 << 4)

void write_uart(char ch);
unsigned char read_uart(void);
void init_uart(void);

#ifdef __cplusplus
}
#endif

