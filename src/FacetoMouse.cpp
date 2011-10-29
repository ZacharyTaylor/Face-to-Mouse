#include "stdafx.h"
#include "FacetoMouse.h"

typedef wchar_t WCHAR ;
typedef CONST WCHAR *LPCWSTR;
wchar_t sound[]=L"C:/Users/Z/Documents/visual studio 2010/Projects/Mouse2/sound95.wav";
wchar_t sound2[]=L"C:/Users/Z/Documents/visual studio 2010/Projects/Mouse2/beep-3.wav";


//calculates where mouse will be positioned
void set_position(Point2f *position, Point2f prevPos, float angle[2], float angleAvg[2], float scale)
{
	position->x = -(angle[0] - angleAvg[0]);
	position->x = scale*(position->x - prevPos.x) + prevPos.x;
	position->y = (angle[1] - angleAvg[1]);
	position->y = scale*(position->y - prevPos.y) + prevPos.y;
}

//clicks the mouse when its been idle
int click_mouse(Point2f position, Point2f prevPos)
{
	static int clickCounter = 0;
	static Point2f ges;
	static int signal = 0;

	if(signal == 1)
	{
		//left for left click
		if(position.x - ges.x < -20)
		{
			PlaySound(sound2, NULL, SND_ASYNC);
			SetCursorPos((int)ges.x + cx, (int)ges.y + cy);
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			signal = 0;
		}
		//right for right click
		if(position.x - ges.x > 20)
		{
			PlaySound(sound2, NULL, SND_ASYNC);
			SetCursorPos((int)ges.x + cx, (int)ges.y + cy);
			mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
			mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
			signal = 0;
		}
		//up for double click
		if(position.y - ges.y < -20)
		{
			PlaySound(sound2, NULL, SND_ASYNC);
			SetCursorPos((int)ges.x + cx, (int)ges.y + cy);
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			signal = 0;
		}
		//down for hold left click
		if(position.y - ges.y > 20)
		{
			PlaySound(sound2, NULL, SND_ASYNC);
			SetCursorPos((int)ges.x + cx, (int)ges.y + cy);
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			signal = 0;
		}

		//hold for nothing
		if(abs(position.x - prevPos.x) + abs(position.y - prevPos.y) < MOUSE_DETECT)
		{
			clickCounter++;
			//if it hasnt moved much in a while click the mouse
			if(clickCounter > 2*CLICK_DETECT)
			{
				PlaySound(sound2, NULL, SND_ASYNC);
				signal = 0;
				clickCounter = 0;
			}
		}
	
	}
	//if the mouse hasnt moved much
	else if((abs(position.x - prevPos.x) + abs(position.y - prevPos.y)) < MOUSE_DETECT)
	{
		clickCounter++;
		//if it hasnt moved much in a while click the mouse
		if(clickCounter > CLICK_DETECT)
		{
			//plays a sound typedefs get round stupid error
			PlaySound(sound, NULL, SND_ASYNC);
			signal = 1;
			clickCounter = 0;
			ges.x = position.x;
			ges.y = position.y;
		}
	}
	//else dont click the mouse
	else
	{
		clickCounter = 0;
	}

	return signal;
}

//ensures mouse stays on screen
void mouse_bounds(Point2f *position)
{
	if(position->x > cx)
	{
		position->x = (float)(cx-EDGE_OFFSET);
	}
	else if(position->x < -cx)
	{
		position->x = -(float)(cx-EDGE_OFFSET);
	}
	if(position->y > cy)
	{
		position->y = (float)(cy-EDGE_OFFSET);
	}
	else if(position->y < -cy)
	{
		position->y = -(float)(cy-EDGE_OFFSET);
	}
}

//updates storage of movements angles
void update_angle_store(float angle[2], float angleOld[2], float angleStore[SCALE_FRAMES][2])
{
	static int n = 0;
	n++;
	if(n > SCALE_FRAMES)
	{
		n = 0;
	}
	angleStore[n][0] = abs(angle[0]- angleOld[0]);
	angleStore[n][1] = abs(angle[1]- angleOld[1]);

	//if the movement is too large take it as a glitch and limit its effect on results
	if(angleStore[n][0] > MAXIMUM_SCALE_MOVEMENT)
	{
		angleStore[n][0] = MAXIMUM_SCALE_MOVEMENT;
	}
	if(angleStore[n][1] > MAXIMUM_SCALE_MOVEMENT)
	{
		angleStore[n][1] = MAXIMUM_SCALE_MOVEMENT;
	}
}

//edge scale factor. This slows mouse down as it gets near edge of screen for easy selection of menus scales scale by between 0 and 1
// the scaling factors are alot more arbitrary then they appear
float edge_scale(Point2f prevPos, float scale)
{
	float power = EDGE_SCALE;
	if(((((cx) - abs(prevPos.x))/cx) < ((cy) - abs(prevPos.y))/cy))
	{
		scale = scale*(((cx) - abs(prevPos.x))/cx);
		return pow(scale,power);
	}
	else
	{
		scale = scale*(((cy) - abs(prevPos.y))/cy);
		return pow(scale,power);
	}
}

//scales mouse sensitivy based on resent movement. Large movements = increased sensitvity
float main_scale(float angleStore[SCALE_FRAMES][2])
{
	float scale = 0;
	float power = BASE_SCALE;
	for(int n = 0; n < SCALE_FRAMES; n++)
	{
		scale += angleStore[n][0]; 
		scale += angleStore[n][1];
	}
	//scales number to between 0 and 4
	scale = (scale/(4*SCALE_FRAMES*MAXIMUM_SCALE_MOVEMENT));
	return pow(scale,power);
}