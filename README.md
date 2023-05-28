# Olympus
Online Multiplayer Pong made in c



### Port Numbers
5454 = Master Server Port
5455 = Slave Master Port

### The chatroom has two main executables

main:
  main is the client executable it has a basic menu with buttons to interface through the program
  
    in the main client you have options single player and multiplayer the rest is pretty straight forward
    
      In the multiplayer option you can be either a client or a server
      when you select clients it will attempt to connect to a master server record where it will list what servers are currently connected
      and avaialable to play with
      
      If you select server it will attempt to connect to a master server record where it will send the information about the server and then     wait for a client to connect to
      
  
server:
  server is the server executable and it is the master server record which will store all information about servers avaiable to clients
  without this you will not be able to play multiplayer
  
### The Pong game

The actual pong game is a simple one with two paddles with variable length and a determinate tickspeed
