#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <jpeg/jpgogc.h>
#include <unistd.h>
#include <math.h>

//Control Flow Definitions
#define MAIN_MENU 1
#define SHOW_SETTINGS 2
#define CONTROLLER_TEST 3
#define ANALOG_CALIBRATION 4
#define CSTICK_CALIBRATION 5
#define TRIGGER_SETUP 6
#define EXIT_APP 7

//Rendering Vars
static u32 *xfb;
static GXRModeObj *rmode;

//Image Vars
extern char picdata[];
extern int  piclength;

//Images
JPEGIMG phost;

//Control Flow Vars
int menu_pointer = 2;
int current_screen = 1;

//Gamepad Vars
u16 buttonsDown;
u16 buttonsHeld;
u16 buttonsUp;

int rTrigger;
int lTrigger;

//Controller Communication Vars
int message_received = 0;
int xDelay = 0;
int yDelay = 0;
int xSnapback = 0;
int ySnapback = 0;

//Stick Display Vars
int rawAx;
int rawAy;
int rawCx;
int rawCy;

float processedAx;
float processedAy;
float processedCx;
float processedCy;

float aVector;
float cVector;

int Ax;
int Ay;
int Cx;
int Cy;

int meleeAx;
int meleeAy;
int meleeCx;
int meleeCy;

/*
* Initialize Function handles setting up the VT Terminal used for printing text
* to the screen and frame buffer for images.
*/
void Initialise() {

	VIDEO_Init();
	PAD_Init();

	rmode = VIDEO_GetPreferredMode(NULL);

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

char *convert(uint8_t *a){
  char* buffer2;
  int i;

  buffer2 = malloc(9);
  if (!buffer2)
    return NULL;

  buffer2[8] = 0;
  for (i = 0; i <= 7; i++)
    buffer2[7 - i] = (((*a) >> i) & (0x01)) + '0';

  puts(buffer2);

  return buffer2;
}

/*
* takes in an image and places it on the screen with a location to put it
*/
void display_jpeg(JPEGIMG jpeg, int x, int y) {

	unsigned int *jpegout = (unsigned int *) jpeg.outbuffer;

	int i,j;
	int height = jpeg.height;
	int width = jpeg.width/2;
		for(i=0;i<=width;i++)
			for(j=0;j<=height-2;j++)
				xfb[(i+x)+320*(j+16+y)]=jpegout[i+width*j];

	free(jpeg.outbuffer);

}

/*
* handles rendering the menu options and the arrow next to them using VT
* Terminal commands.
*/
void display_menu (int men_val) {

	printf("\x1b[10;30H");
	printf("PhobGCC Configuration Tool");

	printf("\x1b[12;35H");
	printf("See Current Controller Settings");

	printf("\x1b[13;35H");
	printf("Controller Test");

	printf("\x1b[14;35H");
	printf("Analog Stick Calibration");

	printf("\x1b[15;35H");
	printf("C-Stick Calibration");

	printf("\x1b[16;35H");
	printf("Trigger Setup");

	printf("\x1b[17;35H");
	printf("Exit");

	if(men_val == 2) {
			printf("\x1b[12;32H");
			printf("->");

	} else if(men_val == 3) {
			printf("\x1b[13;32H");
			printf("->");
	} else if(men_val == 4) {
			printf("\x1b[14;32H");
			printf("->");
	} else if(men_val == 5) {
			printf("\x1b[15;32H");
			printf("->");
	} else if(men_val == 6) {
			printf("\x1b[16;32H");
			printf("->");
	} else if(men_val == 7) {
			printf("\x1b[17;32H");
			printf("->");
	}
}

/*
* handles main menu operation
*/
void main_menu () {

		if(buttonsDown & PAD_BUTTON_START) {
			current_screen = menu_pointer;
		}

		if(buttonsDown & PAD_BUTTON_UP) {
			if(menu_pointer == 2){
			} else {
				menu_pointer--;
			}
		}

		if(buttonsDown & PAD_BUTTON_DOWN) {
			if(menu_pointer == 7){
			} else {
				menu_pointer++;
			}
		}

		display_menu(menu_pointer);
		display_jpeg(phost, 60, 100);
}

void show_settings() {

	if(buttonsDown & PAD_BUTTON_START) {
		current_screen = 1;
	}

	printf("\x1b[2;0H");
	printf("PhobGCC Communication WIP");

			/*	PAD_ScanPads();
				u16 buttonsHeld = PAD_ButtonsHeld(0);
				u16 buttonsDown = PAD_ButtonsDown(0);

				if(message_received == 1) {
					printf("\x1b[2;0H");
					printf("Communicating with PhobGCC");
					usleep(500000); //sleep for half a second

					xDelay = PAD_StickX(0);
					yDelay = PAD_StickY(0);

					xSnapback = PAD_SubStickX(0);
					ySnapback = PAD_SubStickY(0);

					message_received = 2;
				} else if(message_received == 0) {
					printf("\x1b[2;0H");
					printf("Please Send Settings (LR+Start)");
					if ((buttonsHeld & PAD_TRIGGER_L) && (buttonsHeld & PAD_TRIGGER_R) && (buttonsHeld & PAD_BUTTON_START)) {
							message_received = 1;
					}
				} else if (message_received == 2) {

					printf("\x1b[2;0H");
					printf("X axis Delay Setting: %d", xDelay);

					printf("\x1b[3;0H");
					printf("Y Axis Delay Setting: %d", yDelay);

					printf("\x1b[4;0H");
					printf("X axis Snapback Setting: %d", xSnapback);

					printf("\x1b[5;0H");
					printf("Y axis Snapback Setting: %d", ySnapback);

					if(buttonsDown & PAD_BUTTON_START) {
						current_screen = 1;
						message_received = 0;
						break;
					}

				}
				VIDEO_WaitVSync();
				VIDEO_ClearFrameBuffer (rmode, xfb, COLOR_BLACK);*/
}

/*
* handles controller test operation
*/
void controller_test() {
	if(buttonsDown & PAD_BUTTON_START) {
		current_screen = 1;
	}

	printf("\x1b[2;0H");
	printf("Controller Test WIP");
}

int returnIntCast(float val) {
	if(val > 0) {
		return ((int) floor(val));
	} else if(val < 0) {
		return ((int) ceil(val));
	} else {
		return 0;
	}
}
/*
* handles analog calibration operation
*/
void analog_calib() {
		if(buttonsDown & PAD_BUTTON_START) {
			current_screen = 1;
			menu_pointer = 2;
		} else {

		processedAx = rawAx;
		processedAy = rawAy;
		processedCx = rawCx;
		processedCy = rawCy;

		aVector = sqrt((processedAx*processedAx) + (processedAy*processedAy));
		cVector = sqrt((processedCx*processedCx) + (processedCy*processedCy));

		if (aVector > 80) {
			processedAx = (processedAx / aVector) * 80;
			processedAy = (processedAy / aVector) * 80;
		}

		if(cVector > 80) {
			processedCx = (processedCx / cVector) * 80;
			processedCy = (processedCy / cVector) * 80;
		}

		Ax = returnIntCast(processedAx);
		Ay = returnIntCast(processedAy);
		Cx = returnIntCast(processedCx);
		Cy = returnIntCast(processedCy);

		meleeAx = (int) ((((float) (Ax)) * 0.0125) * 10000);
		meleeAy = (int) ((((float) (Ay)) * 0.0125) * 10000);
		meleeCx = (int) ((((float) (Cx)) * 0.0125) * 10000);
		meleeCy = (int) ((((float) (Cy)) * 0.0125) * 10000);

		printf("\x1b[2;0H");
		printf("Raw Analog Stick X Value: %d", rawAx);

		printf("\x1b[3;0H");
		printf("Raw Analog Stick Y Value: %d", rawAy);

		printf("\x1b[4;0H");
		if(rawAx < 0) {
			meleeAx = abs(meleeAx);
			if(meleeAx == 10000) {
				printf("Analog Stick X Value: -1.0000");
			} else {
				printf("Analog Stick X Value: -0.%04d", meleeAx);
			}
		} else {
			if(meleeAx == 10000) {
				printf("Analog Stick X Value: 1.0000");
			} else {
				printf("Analog Stick X Value: 0.%04d", meleeAx);
			}
		}

		printf("\x1b[5;0H");
		if(rawAy < 0) {
			meleeAy = abs(meleeAy);
			if(meleeAy == 10000) {
				printf("Analog Stick Y Value: -1.0000");
			} else {
				printf("Analog Stick Y Value: -0.%04d", meleeAy);
			}
		} else {
			if(meleeAy == 10000) {
				printf("Analog Stick Y Value: 1.0000");
			} else {
				printf("Analog Stick Y Value: 0.%04d", meleeAy);
			}
		}

		printf("\x1b[2;40H");
		printf("Raw C-Stick X Value: %d", rawCx);

		printf("\x1b[3;40H");
		printf("Raw C-Stick Y Value: %d", rawCy);

		printf("\x1b[4;40H");
		if(rawCx < 0) {
			meleeCx = abs(meleeCx);
			if(meleeCx == 10000) {
				printf("C-Stick X Value: -1.0000");
			} else {
				printf("C-Stick X Value: -0.%04d", meleeCx);
			}
		} else {
			if(meleeCx == 10000) {
				printf("C-Stick X Value: 1.0000");
			} else {
				printf("C-Stick X Value: 0.%04d", meleeCx);
			}
		}


		printf("\x1b[5;40H");
		if(rawCy < 0) {
			meleeCy = abs(meleeCy);
			if(meleeCy == 10000) {
				printf("C-Stick Y Value: -1.0000");
			} else {
				printf("C-Stick Y Value: -0.%04d", meleeCy);
			}
		} else {
			if(meleeCy == 10000) {
				printf("C-Stick Y Value: 1.0000");
			} else {
				printf("C-Stick Y Value: 0.%04d", meleeCy);
			}
		}
	}
}
/*
* handles c-stick calibration operation
*/
void cstick_calib() {
		if(buttonsDown & PAD_BUTTON_START) {
			current_screen = 1;
		}

		printf("\x1b[2;0H");
		printf("C-stick Calibration WIP");
}

/*
* handles trigger setup operation
*/
void trigger_setup() {

		if(buttonsDown & PAD_BUTTON_START) {
			current_screen = 1;
		}

		printf("\x1b[2;0H");
		printf("L Trigger Value: %d", lTrigger);

		printf("\x1b[3;0H");
		if(buttonsHeld & PAD_TRIGGER_L) {
			printf("L Trigger Digital Active");
		}

		printf("\x1b[2;40H");
		printf("R Trigger Value: %d", rTrigger);

		printf("\x1b[3;40H");
		if(buttonsHeld & PAD_TRIGGER_R) {
			printf("R Trigger Digital Active");
		}
}


/*
* main execution loop
*/
int main() {

	memset(&phost, 0, sizeof(JPEGIMG));
	phost.inbuffer = picdata;
	phost.inbufferlength = piclength;
	JPEG_Decompress(&phost);
	Initialise();
	display_jpeg(phost, 60, 100);

	while (1) {

		PAD_ScanPads();
		buttonsDown = PAD_ButtonsDown(0);
		buttonsHeld = PAD_ButtonsHeld(0);
		buttonsUp = PAD_ButtonsUp(0);

		rawAx = PAD_StickX(0);
		rawAy = PAD_StickY(0);
		rawCx = PAD_SubStickX(0);
		rawCy = PAD_SubStickY(0);

		lTrigger = PAD_TriggerL(0);
		rTrigger = PAD_TriggerR(0);

		if(current_screen == MAIN_MENU) {
			main_menu();
		} else if(current_screen == SHOW_SETTINGS) {
			show_settings();
		} else if(current_screen == CONTROLLER_TEST) {
			controller_test();
		} else if(current_screen == ANALOG_CALIBRATION) {
			analog_calib();
		} else if(current_screen == CSTICK_CALIBRATION) {
			cstick_calib();
		} else if(current_screen == TRIGGER_SETUP) {
			trigger_setup();
		} else if(current_screen == EXIT_APP) {
			exit(0);
		}

		VIDEO_WaitVSync();
		VIDEO_ClearFrameBuffer (rmode, xfb, COLOR_BLACK);

	}
	return 0;
}
