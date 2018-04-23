# clock constraints
#create_clock -period 3.332 -name sys_clk [get_nets user_si570_sysclk_clk_p]

#FALSE PATH CONSTRAINTS
#Ignore Interrupts of Nyuzi
set_false_path -through [get_pins GPU_BD_i/NYUZI/interrupt*]