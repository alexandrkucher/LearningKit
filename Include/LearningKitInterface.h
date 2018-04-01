#pragma once

#ifndef _LEARNINGKITINTERFACE
#define _LEARNINGKITINTERFACE

#include <windows.h>
#include "Public.h"

typedef enum
{
	Undefined = 0x0,
	Zero = 0xD7,
	One = 0x6,
	Two = 0xB3,
	Three = 0xA7,
	Four = 0x66,
	Five = 0xE5,
	Six = 0xF5,
	Seven = 0x07,
	Eight = 0xF7,
	Nine = 0xE7,
	/*Ten,
	A,
	C,
	E,
	F,
	H,
	L,
	P,
	S,
	U*/

} KIT_SYMBOL;
typedef enum
{
	Empty = 0x0,
	Bar1 = 0x20,
	Bar2 = 0x40,
	Bar3 = 0x80,
	Bar4 = 0x01,
	Bar5 = 0x02,
	Bar6 = 0x04,
	Bar7 = 0x08,
	Bar8 = 0x10
}KIT_BAR;
typedef enum 
{	
	Switch1 = 0x80,
	Switch2 = 0x40,
	Switch3 = 0x20,
	Switch4 = 0x10,
	Switch5 = 0x08,
	Switch6 = 0x04,
	Switch7 = 0x02,
	Switch8 = 0x01,
}KIT_SWITCHES;

__declspec(dllexport) _Bool DisplaySymbol(KIT_SYMBOL symbol);
__declspec(dllexport) KIT_SYMBOL GetDisplayedSymbol();
__declspec(dllexport) _Bool LightBars(KIT_BAR bar);
__declspec(dllexport) KIT_BAR GetLightBars();
__declspec(dllexport) KIT_SWITCHES GetSwitches();
__declspec(dllexport) KIT_SWITCHES GetSwitchesInterrupt();

#endif // !_LEARNINGKITINTERFACE