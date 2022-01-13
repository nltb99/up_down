# A simple console game, implemented using ncurses library.

[![Game](assets/game.png "Game")]()

## Introduction

This game was inspired by "I wanna be the Boshy" created by developer Jesper "Solgryn" Erlandsen. 

For some reason, there was some flicker delay in some Terminal/CMD. Hope you guys find it pleasing.

## Platforms

These platforms are tested to be working.

1. Mac OS X
2. Windows
3. Linux

## Controls:

- __Up-Arrow__ to go up.
- __Down-Arrow__ to go down.
- __Left-Arrow__ to go left.
- __Right-Arrow__ to go right.
- __q__ to exit game. 

## Installing

### macOS

    brew tap nltb99/up_down && brew install up_down

### Linux

    sudo apt-get install up_down

### Windows

    choco install up_down
    
#### Run

    up_down



## Building from source

### Building master

1. Install dependencies:
    * macOS:

            brew install ncurses

    * Window:

            # Install MinGW compiler and follow these instructions
            - https://e-l.unifi.it/pluginfile.php/805205/mod_resource/content/0/ncurses%20installation%20-%20en.pdf
    
    * Linux:

            sudo apt-get install gcc make
            sudo apt-get install libncurses5-dev libncursesw5-dev


            
2. Run the game:

   On MacOS:

        make runmac

    On Windows:

        make runwin

    On Linux:

        make runlinux




```console
                                                                                                       
                                               bbbbbbbb                                                
                  lllllll         tttt         b::::::b               999999999          999999999     
                  l:::::l      ttt:::t         b::::::b             99:::::::::99      99:::::::::99   
                  l:::::l      t:::::t         b::::::b           99:::::::::::::99  99:::::::::::::99 
                  l:::::l      t:::::t          b:::::b          9::::::99999::::::99::::::99999::::::9
nnnn  nnnnnnnn     l::::lttttttt:::::ttttttt    b:::::bbbbbbbbb  9:::::9     9:::::99:::::9     9:::::9
n:::nn::::::::nn   l::::lt:::::::::::::::::t    b::::::::::::::bb9:::::9     9:::::99:::::9     9:::::9
n::::::::::::::nn  l::::lt:::::::::::::::::t    b::::::::::::::::b9:::::99999::::::9 9:::::99999::::::9
nn:::::::::::::::n l::::ltttttt:::::::tttttt    b:::::bbbbb:::::::b99::::::::::::::9  99::::::::::::::9
  n:::::nnnn:::::n l::::l      t:::::t          b:::::b    b::::::b  99999::::::::9     99999::::::::9 
  n::::n    n::::n l::::l      t:::::t          b:::::b     b:::::b       9::::::9           9::::::9  
  n::::n    n::::n l::::l      t:::::t          b:::::b     b:::::b      9::::::9           9::::::9   
  n::::n    n::::n l::::l      t:::::t    ttttttb:::::b     b:::::b     9::::::9           9::::::9    
  n::::n    n::::nl::::::l     t::::::tttt:::::tb:::::bbbbbb::::::b    9::::::9           9::::::9     
  n::::n    n::::nl::::::l     tt::::::::::::::tb::::::::::::::::b    9::::::9           9::::::9      
  n::::n    n::::nl::::::l       tt:::::::::::ttb:::::::::::::::b    9::::::9           9::::::9       
  nnnnnn    nnnnnnllllllll         ttttttttttt  bbbbbbbbbbbbbbbb    99999999           99999999        
                                                                                                      
```
