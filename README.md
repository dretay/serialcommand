# MCP4725 for STM32


Invoke Like this:

```c
	SerialCommand_Command serialcommands[serialcommands_cnt];
	serialcommands[0].command = "dac";
	serialcommands[0].function = &setdac;
	
	SerialCommand_Config serialcommand_config;
	serialcommand_config.command_cnt = serialcommands_cnt;
	serialcommand_config.p_uart = &huart1;
	serialcommand_config.commands = serialcommands;

	SerialCommand.configure(&serialcommand_config, 1);

```
