**INSTRUCTIONS ON HOW TO WRITE NEW OSC FUNCTIONS**

All OSC functions are to be called FunctionNameOSC and places at the bottom of the code. 
If you create a global variable intended to be controlled directly by OSC, place it just above the Setup function, around line 45.

To add an OSC function, add a line to the OSCMsgReceive() function, with the format `msgIN.route("/messagename, FunctionNameOSC);`
Then create a function starting with `void/int/float/etc FunctionNameOSC(OSCMessage &msg, float addrOffset){}`. 
Capitalize each word in the function name for clarity, especially important if it shares a similar name to a variable. 
To access the data in the message, use `msg.getInt(x)/getFloat(x)`, where x is the index of that data in the message (for example, if you are sending a message with two different numbers, you will have `var1 = msg.getInt(0); var2 = msg.getInt(1);`

The second argument of the `OSCMsgReceive()` function is the function that it is routing the message to, so if you change the name of a function, you'll have to change it there, too. 

Each additional OSC function will require a new MAX component, so if possible, combine functionalities into one set of messages. However, it is preferable to add modular controllability to the Arduino and the MAX subpatch than to add a volatile boolean system into the Arduino code, so don't be afraid to put new functions, just add detailed comments (what type of value they're expecting, what sort of range you want that value to be, etc) and I will add it to the MAX patch. 