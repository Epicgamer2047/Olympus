#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>



typedef struct{
	char* text;
	int select;
	int menu;
}button;


char server_h[] = "2425d371eb5cddcf70a683821e9026ab32c26c82";
char client_h[] = "96f846434346e6eca593c7b179b5723a6d6a242a";

typedef struct {
        int socket, flag;
        char ip[15];
        char name[32];
}server_t;



int scrHeight, scrWidth;


void clr_select(button* buttons, int len){
	for(int i=0;i<len;i++){
		buttons[i].select = 0;
	}
}
void printButtons(button* buttons, int len, int selected){
	clr_select(buttons, len);
	for(int i=0;i<len;i++){
		if(i==selected){
			buttons[i].select = 1;
			attron(COLOR_PAIR(2));
			mvprintw(i, 0, buttons[i].text);
			attroff(COLOR_PAIR(2));
		}
		else {
			mvprintw(i,0,buttons[i].text);
		}

	}
}

#define MENU_BUTTONS 3
#define PLAY_BUTTONS 2


void drawPlayer(int y, int x, int len){
	for(int i=y;i<(y+len);i++){
		attron(COLOR_PAIR(2));
		mvprintw(i,x," ");
		attroff(COLOR_PAIR(2));
	}


}



typedef struct{
	int x;
	int y;
	int vy;
	int vx;
}ball;

int calculateBall(ball* sack, int player1, int player2, int len, int cout, int* vel){
	
	//Check if the next step hits one end of the board and if it does
	//calculate whether it hits one of the paddles and if not
	//return based on which player to reward
	//if it does hit the paddle make the bottles velocity opposite to what it is to go the other direction instead;
	if((sack->x + sack->vx) == 0){
		if(!((sack->y + sack->vy) >= player1 && (sack->y + sack->vy) <= player1+len)){
			sack->x = (scrWidth/2);
			sack->y = (scrHeight/2);
			return 2;
		}
		//if(*vel != 1)
		//	(*vel)--;
		sack->vx = sack->vx*-1;
	}
	else if((sack->x + sack->vx) == scrWidth-1){
		if(!((sack->y + sack->vy) >= player2  && (sack->y + sack->vy) <= player2+len)){
			sack->x = (scrWidth/2);
			sack->y = (scrHeight/2);
			return 1;
		}
		//if(*vel != 1)
		//	(*vel)--;
		sack->vx = sack->vx*-1;
	}
	//Now calculate whether on its next turn will hit the floor or ceiling and if so revert its y velocity
	if((sack->y + sack->vy) == scrHeight+1 || (sack->y + sack->vy) == -1){
		sack->vy = sack->vy*-1;
	}
	if(cout == (*vel)){
	//	exit(0);
			//now calculate balls current position based on its velocity after calculations and draw
		sack->y = sack->y + sack->vy;
		sack->x = sack->x + sack->vx;
	}
	mvprintw(sack->y, sack->x, "0");
	//mvprintw(0,0,"sack.y: sack.x: player1: player1+len:")	
}

int calculateOnlineBall(ball* sack, int player1, int player2, int len, int cout, int* vel, int sockfd){
	        //Check if the next step hits one end of the board and if it does
        //calculate whether it hits one of the paddles and if not
        //return based on which player to reward
        //if it does hit the paddle make the bottles velocity opposite to what it is to go the other direction instead;
        int result=0;
	if((sack->x + sack->vx) == 0){
                if(!((sack->y + sack->vy) >= player1 && (sack->y + sack->vy) <= player1+len)){
                        sack->x = (scrWidth/2);
                        sack->y = (scrHeight/2);
                        result = 2;
                }
                //if(*vel != 1)
                //      (*vel)--;
                sack->vx = sack->vx*-1;
        }
        else if((sack->x + sack->vx) == scrWidth-1){
                if(!((sack->y + sack->vy) >= player2  && (sack->y + sack->vy) <= player2+len)){
                        sack->x = (scrWidth/2);
                        sack->y = (scrHeight/2);
                        result = 1;
                }
                //if(*vel != 1)
                //      (*vel)--;
                sack->vx = sack->vx*-1;
        }
        //Now calculate whether on its next turn will hit the floor or ceiling and if so revert its y velocity
        if((sack->y + sack->vy) == scrHeight+1 || (sack->y + sack->vy) == -1){
                sack->vy = sack->vy*-1;
        }
        if(cout == (*vel)){
        //      exit(0);
                        //now calculate balls current position based on its velocity after calculations and draw
                sack->y = sack->y + sack->vy;
                sack->x = sack->x + sack->vx;
        }
	send(sockfd, &sack->y, 4, 0);
	send(sockfd, &sack->x, 4, 0);
        mvprintw(sack->y, sack->x, "0");
	return result;
}

#define speed 6

void gameLocal(){
	
	ball sack;
	sack.x = (scrWidth/2);
	sack.y = (scrHeight/2);
	sack.vy = 1;
	sack.vx = 1;
	
	

	int player1 = (scrHeight/2);
	int player2 = (scrHeight/2);
	int score1 = 0;
	int score2 = 0;

	int length = 11;
		
	int cout = 0;
	int tickSpeed = 7;// ~140 ticks per second
	int ballSpeed = speed; //how many ticks before ball moves

	erase();
	//nodelay(stdscr, true);
	mvprintw(2,10,"Welcome to Local-Play pong press any key to start gets progressively harder");
	mvprintw(3, 12, "Player one uses the Keypad and player two uses [a,d]");
	mvprintw(4, 14, "Press q to exit the entire program press any key to continue");
	getch();
	nodelay(stdscr, true);

	int ch;
	while(1){
		napms(tickSpeed);
		switch(getch()){
			case KEY_DOWN:
				if(player1 != scrHeight)
					player1++;
				break;
			case KEY_UP:
				if(player1 != (scrHeight+length))
					player1--;
				break;
			case 'a':
				if(player2 != (scrHeight))
					player2++;
				break;
			case 'd':
				if(player2 != (scrHeight+length))
					player2--;
				break;
			case 'q':
				nodelay(stdscr, false);
				return;
		}
		erase();	   
	   	drawPlayer(player1, 0, length); //should be at left screen
                drawPlayer(player2, scrWidth-1, length); //should be at right screen
		
		//cout++;
		switch(calculateBall(&sack, player1, player2, length, ++cout, &ballSpeed)){
			case 1:
				score1++;
				ballSpeed = speed;
				break;
			case 2:
				score2++;
				ballSpeed = speed;
				break;
		}	
		mvprintw(0,0,"Player 1 score: %d Player 2 score: %d Speed: %d", score1, score2, ballSpeed);
		if(cout == ballSpeed)
			cout = 0;
	}
	
}

char buffer[1024];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//type button
#define MASTER_SERVER_PORT 5454
#define SLAVE_SERVER_PORT 5455



void deserializeServer(server_t *server, char* buffer){
        memcpy(server->name, buffer, 32);
        buffer += 32;
        memcpy(server->ip, buffer, 15);
}

void gameClient(){
		
      struct sockaddr_in server_addr;
        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        server_t* servers = (server_t*)malloc(sizeof(server_t));

        server_t current;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(MASTER_SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		erase();
		mvprintw(0,0,"Connection to the master server failed please try again later press any key to continue");
		getch();
		return;
	}
        
	send(sockfd, client_h, 20, 0);
        
	//iterate to 100 just to make sure we grabbed all the server info because sometimes it wigs out
	int iteration = 0;
	int serverCount=0;
        while(++iteration<100){
                if(recv(sockfd, buffer, (32+15), 0) > 0){
                        servers = realloc(servers,(++serverCount)*sizeof(server_t));
                        deserializeServer(&current, buffer);
                        servers[serverCount-1] = current;
                        bzero(buffer, 1024);
                }
        }
	
	//initalize the buttons and display all the servers
	//exit(serverCount);	
	
	serverCount++; //make room for the exit button
	
	button buttons[serverCount];
	for(int i=0;i<serverCount-1;i++){
		buttons[i].text = servers[i].name;
	}
	char exit[] = "exit";
	buttons[serverCount-1].text = exit;
	int ch;
        int global_button = 0;
        erase();
        printButtons(buttons,serverCount,global_button);
        while(ch=getch()){
                erase();
		if(ch == KEY_DOWN){
                        if(++global_button == serverCount)
                                global_button =0;
                }
                else if(ch == KEY_UP){
                        if(--global_button < 0)
                                global_button = serverCount-1;
                }
                else if(ch == KEY_ENTER || ch=='\n'){
                	if(global_button == serverCount-1){
				free(servers);
				return;
			}
			int serverfd = socket(AF_INET, SOCK_STREAM, 0);
        		struct sockaddr_in slave_addr;
	       		 slave_addr.sin_family = AF_INET;
		        slave_addr.sin_port = htons(SLAVE_SERVER_PORT);
		        slave_addr.sin_addr.s_addr = inet_addr(servers[global_button].ip);
        		
			
			connect(serverfd, (struct sockaddr*)&slave_addr, sizeof(slave_addr));
			

			int length;
			int lheight;
			int lwidth;
			int player1, player2;
			int x, y;
			int score1, score2;
			recv(serverfd, &length, 4, 0);
			recv(serverfd, &lheight, 4, 0);
			recv(serverfd, &lwidth, 4, 0);
			
			
			nodelay(stdscr, true);

			while(ch = getch()){
				if(ch == 'q'){
					send(serverfd, &ch, 4, 0);
					nodelay(stdscr, false);
					close(serverfd);
					return;
				}
				
				
				send(serverfd, &ch, 4, 0);
				erase();

				if(recv(serverfd, &player1, 4, 0)<0){
					nodelay(stdscr,false);
					close(serverfd);
					return;
				}
				if(recv(serverfd, &player2, 4, 0)<0){
					nodelay(stdscr,false);
                                        close(serverfd);
                                        return;

				}
				drawPlayer(player1, 0, length);
				drawPlayer(player2, lwidth-1, length);
				if(recv(serverfd, &y, 4, 0)<0){
					nodelay(stdscr,false);
                                        close(serverfd);
                                        return;
				}
				if(recv(serverfd, &x, 4, 0)<0){
					nodelay(stdscr,false);
                                        close(serverfd);
                                        return;
				}
				mvprintw(y, x, "0");
				if(recv(serverfd, &score1, 4, 0)<0){
					nodelay(stdscr,false);
                                        close(serverfd);
                                        return;
				}
				if(recv(serverfd, &score2, 4, 0)<0){
					nodelay(stdscr,false);
                                        close(serverfd);
                                        return;
				}
				mvprintw(0,0,"Player 1 score: %d Player 2 score: %d",score1, score2);
			}
		}
                printButtons(buttons, serverCount, global_button);
        }


}



void handle(){
	int server_socket;
 	struct sockaddr_in self_address;

    // Create a server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Set up server socket scheme
    self_address.sin_family = AF_INET;
    self_address.sin_addr.s_addr = INADDR_ANY;
    self_address.sin_port = htons(SLAVE_SERVER_PORT);
    // Bind the server socket to the specified address and port
    
    int reuse = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (bind(server_socket, (struct sockaddr*)&self_address, sizeof(self_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_socket, F_GETFL, 0);

    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

    // Listen for incoming connections
    if (listen(server_socket, 20) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
	
	nodelay(stdscr, true);

	int ch;
	while(ch = getch()){
        	int incoming_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
		if(ch == 'q'){
			nodelay(stdscr, false);
			close(server_socket);
                        close(incoming_socket);
                        return;
                }
		if(incoming_socket == -1)
			continue;
		erase();
		mvprintw(8,10,"Connection recieved press any press any key to continue");
		nodelay(stdscr, false);
		getch();
		

        	ball sack;
        	sack.x = (scrWidth/2);
        	sack.y = (scrHeight/2);
        sack.vy = 1;
        sack.vx = 1;



        int player1 = (scrHeight/2);
        int player2 = (scrHeight/2);
        int score1 = 0;
        int score2 = 0;

        int length = 11;

        int cout = 0;
        int tickSpeed = 3;// ~140 ticks per second
        int ballSpeed = 4; //how many ticks before ball moves

        erase();
        mvprintw(2,10,"Welcome to Multiplayer pong press any key to start");
        mvprintw(3, 12, "Player one and player two both use the keys");
        mvprintw(4, 14, "Press q to exit press any key to continue");
        getch();

	nodelay(stdscr, true);
	int cl;
	int ch;
	
	send(incoming_socket, &length, 4, 0); //send the length of the paddles
	send(incoming_socket, &scrHeight, 4, 0); //send the height of the screen to the client
	send(incoming_socket, &scrWidth, 4, 0); //send the width of the screen to the client
	while(1){
                napms(tickSpeed);
                switch(getch()){
                        case KEY_DOWN:
                                if(player1 != scrHeight)
                                        player1++;
                                break;
                        case KEY_UP:
                                if(player1 != (scrHeight+length))
                                        player1--;
                                break;
                        case 'q':
                                nodelay(stdscr, false);
                                close(server_socket);
                                close(incoming_socket);
                                return;

                }
		if(recv(incoming_socket, &cl, sizeof(cl),0) < 0){
			erase();
			nodelay(stdscr, false);
			mvprintw(3,14,"Player 2 other player has disconnected or quit the game");
			close(server_socket);
                        close(incoming_socket);
                        getch();
			return;

		}
		switch(cl){
			case KEY_DOWN:
				if(player2 != scrHeight)
					player2++;
				break;
			case KEY_UP:
				if(player2 != (scrHeight + length))
					       player2--;
				break;
			case 'q':
				erase();
	                        nodelay(stdscr, false);
        	                mvprintw(3,14,"Player 2 other player has disconnected or quit the game");
                	        close(server_socket);
                        	close(incoming_socket);
                        	getch();
                        	return;
		}
		
                erase();
                drawPlayer(player1, 0, length); //should be at left screen
                drawPlayer(player2, scrWidth-1, length); //should be at right screen
		
		send(incoming_socket, &player1, 4, 0);
		send(incoming_socket, &player2, 4, 0);
		
                //cout++;
                switch(calculateOnlineBall(&sack, player1, player2, length, ++cout, &ballSpeed, incoming_socket)){
                        case 1:
                                score1++;
                  //              ballSpeed = speed;
                                break;
                        case 2:
                                score2++;
                    //            ballSpeed = speed;
                                break;
                }
                mvprintw(0,0,"Player 1 score: %d Player 2 score: %d Speed: %d", score1, score2, ballSpeed);
                send(incoming_socket, &score1, 4, 0);
		send(incoming_socket, &score2, 4, 0);
		if(cout == ballSpeed)
                        cout = 0;
       }
	

}
}

//maybe in future make the server creation much more interactive
//with form.h and fields but not rn fuck dat shit
void gameServer(){
	char ip[15];
	char name[32];
	erase();
	mvprintw(0,0,"Please type in the name of the server: ");
	getnstr(name, 32);
	mvprintw(0, 39, name, 32);
	mvprintw(1,0,"Please type in the ip of your server ex.[loopback local for local connection on the machine: 127.0.0.1]: ");
	getnstr(ip, 15);
	mvprintw(1, 105, ip, 32);
	mvprintw(2,0,"If these are not the correct values press q to return to the menu to try again if not press y");
	int ch = getch();
	if(ch == 'q')
		return;
	int leave = 0;
	erase();
	mvprintw(3,14,"Connecting to Master Server");
	struct sockaddr_in server_addr;
        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(MASTER_SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


        if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		mvprintw(4,16,"Unable to connect to master server press any key to return to menu");
		getch();
		return;
	}
        //registering server
	mvprintw(4,16,"connected to master");
        mvprintw(5,16,"getting server registered\n");
	//sending to tell it is a server
        send(sockfd, server_h, 20, 0);
	//sending to tell that it wants to register its server on the master record
        send(sockfd, &leave, 4, 0);
	//sending its meta data to be stored in the master record       
        send(sockfd, name, 32, 0);
        send(sockfd, ip, strlen(ip), 0);
	close(sockfd);
	mvprintw(8, 16, "Now waiting for player 2 if you want to quit now press q if not wait for the next player to begin playing");	
	
	handle();		

		erase();        
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
        		leave = 1;
			mvprintw(2,16,"Connecting to master server");
		        if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
				erase();
				mvprintw(3,16,"Connection to master server failed press any key to continue to main menu");
				getch();
				return;			
			}
	        	mvprintw(3, 16,"Connected to master");
		        mvprintw(4, 16,"unregistering server");
	      	  	send(sockfd, server_h, 20, 0);
		        send(sockfd, &leave, 4, 0);
	       	 	send(sockfd, name, 32, 0);
		        mvprintw(5,16,"successfully deregisted press any key to continue to the main menu");
	       		close(sockfd);
			getch();
			return;
}


void handleLocal(){
	char first[] = "Enter local game";
	char exit[] = "exit";
	button buttons[PLAY_BUTTONS];
	
	buttons[0].text = first;
	buttons[1].text = exit;
	int ch;
	int global_button = 0;
	erase();
	printButtons(buttons,PLAY_BUTTONS,global_button);
	while(ch=getch()){
		erase();
		if(ch == KEY_UP){
			if(++global_button == PLAY_BUTTONS)
				global_button =0;
		}
		else if(ch == KEY_DOWN){
			if(--global_button < 0){
				global_button = PLAY_BUTTONS-1;
			}
		}
		else if(ch == KEY_ENTER || ch=='\n'){
			if(global_button == 1){
				return;
			}
			gameLocal();
			erase();
		}
	        printButtons(buttons, PLAY_BUTTONS, global_button);

	}
}
	

#define MULTI_BUTTONS 3

void handleMultiplayer(){
	//set up tcp and determine whether this will be a server or a client
	char first[] = "Client";
        char second[] = "Server";
	char exit[] = "exit";

        button buttons[MULTI_BUTTONS];

        buttons[0].text = first;
        buttons[1].text = second;
	buttons[2].text = exit;
        int ch;
        int global_button = 0;
        erase();
        printButtons(buttons,MULTI_BUTTONS,global_button);
        while(ch=getch()){
                erase();
                if(ch == KEY_DOWN){
                        if(++global_button == MULTI_BUTTONS)
                                global_button =0;
                }
                else if(ch == KEY_UP){
                        if(--global_button < 0){
                                global_button = MULTI_BUTTONS-1;
                        }
                }
                else if(ch == KEY_ENTER || ch=='\n'){
                        if(global_button == 2){
                                return;
                        }
			else if(global_button == 1){
				gameServer();
			}
			else if(global_button == 0){
                        	gameClient();
			}
                        erase();
                }
                printButtons(buttons, MULTI_BUTTONS, global_button);
        }

}


void handleMenu(button* menuButtons){
	int global_button = 0;
	erase();
	printButtons(menuButtons, MENU_BUTTONS, global_button);
	int ch;
	while((ch = getch()) != 'q'){

        if(ch == KEY_DOWN){
                if(++global_button == MENU_BUTTONS){
                        global_button = 0;
                }
        }
        else if(ch == KEY_UP){
         	if(--global_button < 0){
                        global_button = MENU_BUTTONS-1;
                }
        }
	else if(ch == KEY_ENTER || ch == '\n'){
		if(menuButtons[global_button].menu == 1){
			handleLocal();
		}
		else if(menuButtons[global_button].menu ==2){
			handleMultiplayer();
		}
		else if(menuButtons[global_button].menu == 0){
			erase();
			endwin();
			exit(0);
		}
	}
	erase();
        printButtons(menuButtons, MENU_BUTTONS, global_button);
}
}




int main(){

char first[] = "Local-Play";
char second[] = "Online-Multiplayer";
char third[] = "exit";

button menuButtons[MENU_BUTTONS];
menuButtons[0].text = first;
menuButtons[0].menu = 1;
menuButtons[1].text = second;
menuButtons[1].menu = 2;
menuButtons[2].text = third;
menuButtons[2].menu = 0;

initscr();
noecho();
cbreak();
start_color();
init_pair(1, COLOR_WHITE, COLOR_BLACK);
init_pair(2, COLOR_BLACK, COLOR_WHITE);
keypad(stdscr, true);
noecho();
curs_set(0);
getmaxyx(stdscr, scrHeight, scrWidth);


handleMenu(menuButtons);



}
