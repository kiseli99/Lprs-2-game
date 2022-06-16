
///////////////////////////////////////////////////////////////////////////////
// Headers.

#include <stdint.h>
#include "system.h"
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// HW stuff.

#define WAIT_UNITL_0(x) while(x != 0){}
#define WAIT_UNITL_1(x) while(x != 1){}


#define SCREEN_IDX4_W 320
#define SCREEN_IDX4_H 240
#define SCREEN_RGB333_W 160
#define SCREEN_RGB333_H 120
#define SCREEN_W 640
#define SCREEN_H 480

#define gpu_p32 ((volatile uint32_t*)LPRS2_GPU_BASE)
#define palette_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x1000))
#define unpack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x400000))
#define pack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x600000))
#define joypad_p32 ((volatile uint32_t*)LPRS2_JOYPAD_BASE)

#define unpack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x800000))
#define pack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xa00000))
#define unpack_rgb333_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xc00000))

typedef struct {
	unsigned a      : 1;
	unsigned b      : 1;
	unsigned z      : 1;
	unsigned start  : 1;
	unsigned up     : 1;
	unsigned down   : 1;
	unsigned left   : 1;
	unsigned right  : 1;
	
} bf_joypad;
#define joypad (*((volatile bf_joypad*)LPRS2_JOYPAD_BASE))

typedef struct {
	uint32_t m[SCREEN_H][SCREEN_W];
} bf_unpack_idx1;
#define unpack_idx1 (*((volatile bf_unpack_idx1*)unpack_idx1s_p32))



///////////////////////////////////////////////////////////////////////////////
// Game config.

#define STEP 4
#define PADDLE_H 16		//playboard size
#define PADDLE_W 128
#define OBSTACLE_H 16
#define OBSTACLE_W 128
#define GAME_BALL 16

#define UNPACKED_0_PACKED_1 0



///////////////////////////////////////////////////////////////////////////////
// Game data structures.

typedef struct {
	uint16_t x;
	uint16_t y;
} point_t;


typedef struct {
	point_t paddle;
} game_state_t;


typedef enum {
	LEFT,
	RIGHT
}direction_paddle;

//ball
typedef struct {
	point_t gameball;
} ball_state_t;



typedef enum {
	TLeft,
	TRight,
	TUp,
	TDown
}direction_game_ball;



///////////////////////////////////////////////////////////////////////////////
// Game code.

int main(void) {
	
	// Setup.
	gpu_p32[0] = 1; // 1b index mode.
	gpu_p32[1] = UNPACKED_0_PACKED_1;
	palette_p32[0] = 0x00000000; // Black for background.
	palette_p32[1] = 0x000000ff; // Red for paddle and ball
	gpu_p32[0x800] = 0x0000ff00; // Green for HUD.


	// Game state.
	game_state_t gs;
	ball_state_t bs;
	direction_paddle dir;
	direction_game_ball direction;
	//starting position of the racket	
	gs.paddle.x = 0;
	gs.paddle.y = 465;
	
	//the initial position of the ball
	bs.gameball.x = 256;
	bs.gameball.y = 256;
	int x = -2;	//moving the ball along the x axis
	int y = 2;	//moving the ball along the y axis
	float k = 0;
	
	while(1){
		/////////////////////////////////////
		// Poll controls.
		
		int mov_x = 0;
		int mov_y = 0;
		int move_ball_x = 0;
		int move_ball_y = 0;
		//moving racket 
		if(joypad.right){
			mov_x = +1;
			dir = RIGHT;
			
		}
		if(joypad.left){
			mov_x = -1;
			dir = LEFT;
			}
		
		
		//restart game
		if(joypad.a){
		bs.gameball.x = 256;
		bs.gameball.y = 256;
		gs.paddle.x = 0;
		gs.paddle.y = 465;
		x = -2;
		y = 2;
		}
		//acceleration
		if(joypad.z){
		k-=0.3;
		//deceleration
		}
		if(joypad.b){
		k+=0.3;
		}
		
		
		/////////////////////////////////////
		// Gameplay.
		//moving the racket through the wall
		gs.paddle.x += mov_x*(STEP+k);
		bs.gameball.x += move_ball_x*STEP;
		bs.gameball.y += move_ball_y*STEP;
		
		
		if(gs.paddle.x > SCREEN_W && dir == RIGHT)
		{
			gs.paddle.x -= SCREEN_W;
		}
		if(gs.paddle.x < 0 && dir == LEFT)
		{
			gs.paddle.x = 0;
		}
		
		
		
		
		if((bs.gameball.x >= gs.paddle.x && bs.gameball.x <= gs.paddle.x + 128))
	
		{
		
		if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x < SCREEN_W-GAME_BALL && x == 2 && y == 2 && dir == 0) {
			//touch bottom edge from left paddle dont move 
			x = 2;
			y = -2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x < SCREEN_W-GAME_BALL && x == 2 && y == 2 && dir == LEFT) {
			if(bs.gameball.x == gs.paddle.x){
			//touch bottom edge from left paddle go left 
			x = 2;
			y = -2;
		}
		else{bs.gameball.x = 0;
			bs.gameball.y = 0;}
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x == bs.gameball.x && x == 2 && y == 2 && dir == RIGHT) {
			//touch paddle from left side and paddle go right 
			x = -2;
			y = -2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x < SCREEN_W-GAME_BALL && x == -2 && y == 2 && dir == LEFT) {		
			//touch bottom edge from right side paddle go left 
			x = 2;
			y = -2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x < SCREEN_W-GAME_BALL && x == -2 && y == 2 && dir == RIGHT) {		
			//touch bottom edge from right side paddle go right 
			x = -2;
			y = -2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL-PADDLE_H && bs.gameball.x < SCREEN_W-GAME_BALL && x == -2 && y == 2) {		
			//touch bottom edge from right side paddle dont move 
			x = -2;
			y = -2;
		}
		/*
		else if (bs.gameball.y == SCREEN_H-GAME_BALL && bs.gameball.x == SCREEN_W-GAME_BALL){
		x = 0;
		y = 0;}*/
	
		}
		else if(bs.gameball.y < SCREEN_H-GAME_BALL && bs.gameball.x == SCREEN_W-GAME_BALL && x == 2 && y == -2) {
			//touch right edge from bellow 
			x = -2;
			y = -2;
		}
		if(bs.gameball.y < SCREEN_H-GAME_BALL && bs.gameball.x == SCREEN_W-GAME_BALL && x == 2 && y == 2) {
			//touch right edge from above
			x = -2;
			y = 2;
		}
		else if(bs.gameball.y == 0 && bs.gameball.x < SCREEN_W-GAME_BALL && x == -2 && y == -2) {
			//touch upper edge from right 
			x = -2;
			y = 2;
		}
		else if(bs.gameball.y == 0 && bs.gameball.x < SCREEN_W-GAME_BALL && x == 2 && y == -2) {
			// touch upper edge from left
			x = 2;
			y = 2;
		}
		else if(bs.gameball.y < SCREEN_H-GAME_BALL && bs.gameball.x == 0 && x == -2 && y == -2) {
			//touch left edge from above 
			x = 2;
			y = -2;
		}
		else if(bs.gameball.y < SCREEN_H-GAME_BALL && bs.gameball.x == 0 && x == -2 && y == 2) {
			//touch left edge from below
			x = 2;
			y = 2;
		}
	
			//if ball go to corners
		else if(bs.gameball.y == GAME_BALL && bs.gameball.x == GAME_BALL && x == -2 && y == -2) {
			//top left
			x = 2;
			y = 2;
		}
		else if(bs.gameball.y == 0 && bs.gameball.x == SCREEN_W-GAME_BALL && x == 2 && y == -2) {
			//top right
			x = -2;
			y = 2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL && bs.gameball.x == 0 && x == -2 && y == 2) {
			//bottom left
			x = 2;
			y = -2;
		}
		else if(bs.gameball.y == SCREEN_H-GAME_BALL && bs.gameball.x == SCREEN_W-GAME_BALL && x == 2 && y == 2) {
			//bottom right
			x = -2;
			y = -2;
		}
		
			bs.gameball.x += x;
			bs.gameball.y += y;
			
	
		
		
		
		
		
		
		
		
		/////////////////////////////////////
		// Drawing.
		
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync.
		
		
		
//#if !UNPACKED_0_PACKED_1
		// Unpacked.
		
		
		
		// Clear to background.
		for(int r = 0; r < SCREEN_H; r++){
			for(int c = 0; c < SCREEN_W; c++){
				unpack_idx1_p32[r*SCREEN_W + c] = 0;
			}
		}
		
		
		// Red paddle
		// Use array with 2D indexing.
		for(int r = gs.paddle.y; r < (gs.paddle.y+PADDLE_H); r++){
			for(int c = gs.paddle.x; c < (gs.paddle.x+PADDLE_W); c++){
				unpack_idx1_p32[r*SCREEN_W + c] = 1;
			}
		}
		

		// game ball
		// Use struct with 2D matrix.
		for(int r = bs.gameball.y; r < (bs.gameball.y+GAME_BALL); r++){
			for(int c = bs.gameball.x; c < (bs.gameball.x+GAME_BALL); c++){
				//unpack_idx1.m[r][c] = 5;
				unpack_idx1_p32[r*SCREEN_W + c] = 1;
			}
		}
		/*draw_sprite(
			red__p  [gs.digits[3]],32, 64, 32+(3-3)*40, 32
		);
		draw_sprite(
			red__p  [gs.digits[2]], 32, 64, 32+(3-2)*40, 32
		);
		
		*/
		
//#else
//#endif*
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
