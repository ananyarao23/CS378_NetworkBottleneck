## Compiling `sender.c`
- Compile using the command `g++ -o sender sender.c`
- Execute uisng the command `./sender <Packet Size> <IP Address> <time-space> <number of packet pairs>`
- Make sure the receiver program is already running when you execute

## Compiling `receiver.c`
- Compile using the command `g++ -o receiver receiver.c`
- Execute using the command `./receiver <output file name>`
- This program outputs some debug statements on STDOUT, like the messages, time recorded, pair identifiers. It can be redirected to another file using `>`. The given output file will comprise of the recorded time values only, with each value on a new line

Make sure the port `8080` is free before running the experiments. If not, please change the port numbers in both the programs accordingly.

