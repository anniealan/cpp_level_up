### Implement a task manager which uses FIFOs for communication. 

#### There should be a main input FIFO created by the manager which is used to pass commands to be executed in child processes. Each task must create a new FIFO (the name is a subject of a command parameter) to send the output which must be read by the client.