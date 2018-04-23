#
# Copyright 2011-2017 Jeff Bush
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

TOPDIR=../../../
OUTPUT=cup
MODEL_FILE=res/mug.obj

include $(TOPDIR)/build/target.mk
HEX2BIN=$(TOPDIR)/tools/misc/hex2bin.py

all: $(OUTPUT).bin

clean:
	rm -rf $(OBJ_DIR)
	rm -f *.bin

$(OUTPUT).bin:
	python make_resource_file.py $(MODEL_FILE)
	mv resource.bin $(OUTPUT).bin

-include $(DEPS)

