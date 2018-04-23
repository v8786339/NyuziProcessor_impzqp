#ifndef SRC_SHARED_MEM_ITF_HPP_
#define SRC_SHARED_MEM_ITF_HPP_

#define BASE_ADDR 	 0x400000000
#define UPPER_ADDR	 0x41ffffff

typedef struct {
	uint32_t fps;
	uint32_t status;
} smdb_t;

// shared memory data base
#define SMDB_START_ADDRESS 0x41fffffe0

#endif /* SRC_SHARED_MEM_ITF_HPP_ */

