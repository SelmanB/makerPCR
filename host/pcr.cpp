#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#define STEP 4


//GLOBALS
FILE* dev;
char argdef[3][15]={"/dev/ttyACM0","9600","/dev/video1"};
int time;

void alarmHandler(int number){
	alarm(STEP);
	time+=STEP;
}


/***CONTROL FUNCTIONS***/
void led(bool on){
	char command=on?'1':'0';
	fputc('3',dev);
	fflush(NULL);
	fputc(command,dev);
	fflush(NULL);
}
int photo(const char* camera,const char* filename){
	char com[100]={0};
	strcat(com,"fswebcam -d ");
	strcat(com,camera);
	strcat(com," ");
	strcat(com,filename);
	led(true);
	int stat= system(com);
	led(false);
	return stat;
}
int photo(const char* camera){
	char fn[100]={0};
	sprintf(fn,"%d.jpg",time);
	photo(camera,fn);
}
void vertical(bool up){
	char command=up?60:120;
	fputc('5',dev);
	fputc(command,dev);
	fflush(NULL);
}
void horizontal(int slot){//slot \in {0,1,2}
	char command=(char)(2-slot)*60;
	fputc('4',dev);
	fputc(command,dev);
	fflush(NULL);
}

void setT(char t0, char t1, char t2){
	fputc('6',dev);
	fputc(t0,dev);
	fputc('7',dev);
	fputc(t1,dev);
	fputc('8',dev);
	fputc(t2,dev);
	fflush(NULL);
}
void change(int slot){
	vertical(true);
	horizontal(slot);
	vertical(false);
}




void init(char **argv){ //microcontroller directory and baud rate
	char message[200]={0};
	strcat(message,"stty -F ");
	strcat(message,argv[1]);
	strcat(message," cs8 ");
	strcat(message,argv[2]);
	strcat(message," ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts");
	int res=system(message);
	if(res){
		perror("Problem setting stty for the device\n");
		exit(-1);
	}
	dev=fopen(argv[1],"w");	
	if(!dev){
		perror("Device cannot be accessed");
		exit(-1);
	}
	puts("Initialization done.");
}

void terminate(){
	fclose(dev);
}


int main(int argc,char **argv){//device directory (including /dev) and baud-rate and camera directory OR none
	char* def[4]={NULL,&argdef[0][0],&argdef[1][0],&argdef[2][0]};
	if(argc==1)init(def);
	else if(argc==4){
		init(argv);
		def[3]=argv[3];
	} else {
		fputs("Arguments should be empty OR slave device path, baud rate and camera path.\n",stderr);
		exit(-1);
	}


	signal(SIGALRM,alarmHandler);
	alarm(STEP);

	
//***THIS PART CAN BE MODIFIED TO GET DIFFERENT BEHAVIOR***
	setT(35,40,45);
	bool on =true;
	while(time<500){
		switch((time/STEP)%13){
			case 0:
				vertical(true);
				led(true);
			break;
			case 1:
				photo(def[3]);
				led(false);
				change (1);
			break;
			case 5:
				change(2);
			break;
			case 9:
				change(0);
			break;
			default:
			break;
		}
		pause();
	}

//***********************************************************

	terminate();
	return 0;
}
