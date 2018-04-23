/*
 * shared_mem_itf.hpp - Define memory addresses and shared data structure for Nyuzi
 *
 *      Course: SoC Design Laboratoy, 2017W
 *      Author: Edwin Willegger (1326324), Markus Kessler (1225380)
 *      Mail:   [matrNr]@student.tuwien.ac.at
 */

#ifndef SRC_SHARED_MEM_ITF_HPP_
#define SRC_SHARED_MEM_ITF_HPP_

// ATTENTION: Nyuzi uses 32-bit for memory addressing.
// Therefore, the following addresses are casted to uint32_t and the higher 32-bits are removed.
// For example, 0x41fffffe0 would be interpreted as 0x1fffffe0.
// The prefix 0x04 is added to all outgoing AXI connections in Nyuzis HDL-Wrapper.
#define SMDB_ADDR   0x41fffffe0

#define ACTION_BIT_ROTATE 	0
#define ACTION_BIT_LEFT 	1
#define ACTION_BIT_RIGHT 	2
#define ACTION_BIT_UP 		3
#define ACTION_BIT_DOWN 	4

/* The shared memory database is used to exchange data between the PS and PL system */
typedef struct {
    uint32_t program_fil_st_addr;
    uint32_t program_fil_size;
    uint32_t resource_fil_st_addr;
    uint32_t resource_fil_size;
    uint32_t frame_rate;
    /**
     * bit 1: If set, rotation mode is active. Otherwise, movement mode is selected
     * bit 2: Direction left
     * bit 3: Direction right
     * bit 4: Direction up
     * bit 5: Direction down
     */
    uint32_t camera_actions;
} smdb_t;

#endif /* SRC_SHARED_MEM_ITF_HPP_ */

