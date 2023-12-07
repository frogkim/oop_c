# oop_c
This project will be a part of trading_plaform librires (https://github.com/frogkim/trading_platform/tree/main/src/libnetwork).

This project is a network library. It has two main structure, Server and Client.
The goal of this project is to seperate its internal functions with external functions.
This program is written by C and Windows API. I tried to implement encapsulation compile code seprately and provide limited header to users.

![Structure](https://github.com/frogkim/pictures/blob/main/oop_c_01.png)  
Left is public members of server structure. A user of this library can manage only these for functions.
Right is the actual form of server structure. Prefix '_' indicates the variables are only used in internal functions.

![Constructor](https://github.com/frogkim/pictures/blob/main/oop_c_02.png)  
Constructor cannot be a member of structure, because when a structure is created, initialization should be performed by user.
When a constructor be a member of structure, it will be void functor. I needed to create the contructor outside of the structure.

Constructor returns limited structure pointer 'p_Server_t'. However, inside the constructor, it creates the original structure 'Server_original_t'.
'Server_t' is a subset of 'Server_original_t'. The order must be the same each other. Computer only reads the memory, so if its order is changed, the program understands the function following its order.
It ignores the names of the member.
After the constructor's work is done, it returns original structure 'Server_orignal_t' as limited structer 'Server_t'.

It is very similar with inheritance of OOP. In actually it creates an object as child, it returns the form as its parent.
User cannot access original form's members because I only provide limited header and lib or dll files.
However, I cannot block them to access the memory directly. It is limited only C language's level.
