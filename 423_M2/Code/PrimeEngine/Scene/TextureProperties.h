#ifndef __PYENGINE_2_0_TEXTUREPROPERTIES_H__
#define __PYENGINE_2_0_TEXTUREPROPERTIES_H__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>


enum TextureList {
	FIRE_1,
	FLARE,
	GHOST,
	FIRE_4,
	BLUEY_ORB_THING,
	FIRE_6,
	BALL_2,
	BLUE_EXPLOSION,
	EXPLOSION,
	BUBBLE,
	MUSICAL_NOTE,
	SNOWFLAKES,
	KOFFING
};
struct TextureProperties {
	char texture_name[100];
	float width;
	float height;
	int num_cols;
	int num_rows;
	bool isAlphaBlended;
	TextureProperties() {
		strcpy(texture_name, "fire_1.dds");
		width = 32;
		height = 32;
		num_cols = 1;
		num_rows = 1;
		isAlphaBlended = false;
	}

	TextureProperties(char _texture_name[100], float _width, float _height) {
		strcpy(texture_name, _texture_name);
		width = _width;
		height = _height;
	}
	TextureProperties(TextureList tex) {
		switch (tex) {
		case FIRE_1:
			strcpy(texture_name, "fire_1.dds");
			width = 729;
			height = 1024;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = true;
			break;
		case FLARE:
			strcpy(texture_name, "flare.dds");
			width = 256;
			height = 256;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = false;
			break;
		case GHOST:
			strcpy(texture_name, "ghost.dds");
			width = 781;
			height = 811;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = true;
			break;
		case FIRE_4:
			strcpy(texture_name, "fire_4.dds");
			width = 1024;
			height = 1024;
			num_rows = 4;
			num_cols = 4;
			isAlphaBlended = true;
			break;
		case BLUEY_ORB_THING:
			strcpy(texture_name, "bluey_orb_thing.dds");
			width = 236;
			height = 236;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = true;
			break;
		case FIRE_6:
			strcpy(texture_name, "fire_6.dds");
			width = 250;
			height = 250;
			num_rows = 2;
			num_cols = 2;
			isAlphaBlended = true;
			break;
		case BALL_2:
			strcpy(texture_name, "ball_2.dds");
			width = 500;
			height = 500;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = false;
			break;
		case BLUE_EXPLOSION:
			strcpy(texture_name, "blue_explosion.dds");
			width = 640;
			height = 640;
			num_rows = 4;
			num_cols = 4;
			isAlphaBlended = true;
			break;
		case EXPLOSION:
			strcpy(texture_name, "explosion.dds");
			width = 225;
			height = 225;
			num_rows = 4;
			num_cols = 4;
			isAlphaBlended = true;
			break;
		case BUBBLE:
			strcpy(texture_name, "bubble.dds");
			width = 552;
			height = 552;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = true;
			break;
		case MUSICAL_NOTE:
			strcpy(texture_name, "musical_note.dds");
			width = 222;
			height = 235;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = false;
			break;
		case SNOWFLAKES:
			strcpy(texture_name, "snowflakes.dds");
			width = 640;
			height = 175;
			num_rows = 1;
			num_cols = 3;
			isAlphaBlended = false;
			break;
		case KOFFING:
			strcpy(texture_name, "koffing.dds");
			width = 660;
			height = 660;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = false;
			break;
		default:
			strcpy(texture_name, "fire_3.dds");
			width = 729;
			height = 1024;
			num_rows = 1;
			num_cols = 1;
			isAlphaBlended = true;
			OutputDebugStringA("TextureProperties: You are in the default case. INVALID Texture id.");
			break;
		}
	}
};
#endif