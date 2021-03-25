#include "PPU.h"
#include <iomanip>

void PPU::powerUp() {
	PPUCTRL = 0x00;
	PPUMASK = 0x00;
	PPUSTATUS = 0xA0;
	OAMADDR = 0x00;
	PPUADDR = 0x00;
	PPUDATA = 0x00;
}

uint8_t PPU::IOread(uint16_t addr) {
	switch (addr) {
	case 0x2002: //PPUSTATUS
		w = false;
		return PPUSTATUS;
	case 0x2004: //OAMDATA
		return primaryOAM[OAMADDR];
	case 0x2007: //PPUDATA
		int hold = v;
		if (((PPUCTRL >> 2) & 0x01) == 0) {  //VRAM addr inc per CPU R/W of PPUDATA (0: +1, going across; 1: +32, going down)
			v++;
		}
		else {
			v += 32;
		}
		if (hold < 0x2000) { //pattern teble 0+1: 0000-0FFF, 1000-1FFF
			return m_unit.readCHR(hold);
		}
		else if (hold < 0x3F00) {
			uint16_t index = hold & 0x3FF;
			//DO LATER: COME UP WITH BETTER MIRRORING SCHEME
			if (m_unit.mirroring() == 0) { //horizontal
				if (hold < 0x2800) { //nametable 0+1: 2000-23FF, 2400-27FF
					return ciRAM[index];
				}
				else { //nametable 2+3: 2800-2BFF, 2C00-2FFF
					return ciRAM[0x400 + index];
				}
			}
			else { //vertical
				if (hold < 0x2400 || (hold >= 0x2800 && hold < 0x2C00)) { //nametable 0+2: 2000-23FF, 2800-2BFF 
					return ciRAM[index];
				}
				else if (hold < 0x2800 || (hold >= 0x2C00 && hold < 0x3000)) { //nametable 1+3: 2400-27FF, 2C00-2FFF
					return ciRAM[0x400 + index];
				}
			}
		}
		else if (hold < 4000) { //Palette RAM indexes: 3F00-3F1F, then mirrored until 3FFF
			return paletteRAM[hold & 0x1F];
		}
	}
}

void PPU::IOwrite(uint16_t addr, uint8_t data) {
	switch (addr) {
	case 0x2000: //PPUCTRL
		PPUCTRL = data;
		t &= 0x73FF;
		t |= ((PPUCTRL & 0x03) << 10); //0111 NN11 1111 1111
		break;
	case 0x2001: //PPUMASK
		PPUMASK = data;
		break;
	case 0x2003: //OAMADDR
		OAMADDR = data;
		break;
	case 0x2004: //OAMDATA
		primaryOAM[OAMADDR] = data;
		OAMADDR++;
		break;
	case 0x2005: //PPUSCROLL
		if (!w) {
			PPUSCROLL &= 0x00FF;
			PPUSCROLL |= data << 8;
			t &= 0x7FE0;
			t |= (data >> 3); //0111 1111 111X XXXX
			x = data & 0x07; 
			w = true;
		}
		else {
			PPUSCROLL &= 0xFF00;
			PPUSCROLL |= data;
			t &= 0x0C1F;
			t |= ((data >> 3) << 5);
			t |= ((data & 0x07) << 12); //0yyy 11YY YYY1 1111
			w = false;
		}
		break;
	case 0x2006: //PPUADDR
		if (!w) {
			PPUADDR &= 0x00FF;
			PPUADDR |= data << 8;
			t &= 0x00FF;
			t |= ((data & 0x7F) << 8);
			w = true;
		}
		else {
			PPUADDR &= 0xFF00;
			PPUADDR |= data;
			t &= 0xFF00;
			t |= data;
			v = t;
			w = false;
		}
		break;
	case 0x2007: //PPUDATA
		PPUDATA = data;
		if (v < 0x1000) { //pattern table 0: 0000-0FFF
			//DO LATER
		}
		else if (v < 0x2000) { //pattern teble 1: 1000-1FFF
			//DO LATER
		}
		else if (v < 0x3F00) {
			uint16_t index = v & 0x3FF;
			//DO LATER: COME UP WITH BETTER MIRRORING SCHEME
			if (m_unit.mirroring() == 0) { //horizontal
				if (v < 0x2800) { //nametable 0+1: 2000-23FF, 2400-27FF
					ciRAM[index] = data;
				}
				else { //nametable 2+3: 2800-2BFF, 2C00-2FFF
					ciRAM[0x400 + index] = data;
				}
			}
			else { //vertical
				if (v < 0x2400 || (v >= 0x2800 && v < 0x2C00)) { //nametable 0+2: 2000-23FF, 2800-2BFF 
					ciRAM[index] = data;
				}
				else if (v < 0x2800 || (v >= 0x2C00 && v < 0x3000)) { //nametable 1+3: 2400-27FF, 2C00-2FFF
					ciRAM[0x400 + index] = data;
				}
			}
		}
		else if (v < 0x4000) { //Palette RAM indexes: 3F00-3F1F, then mirrored until 3FFF
			paletteRAM[v & 0x1F] = data;
			if ((v & 0x1F) == 0) {
				paletteRAM[16] = paletteRAM[0];
			}
			else if ((v & 0x1F) == 0x10) {
				paletteRAM[0] = paletteRAM[16];
			}
		}
		if (((PPUCTRL >> 2) & 0x01) == 0) {  //VRAM addr inc per CPU R/W of PPUDATA (0: +1, going across; 1: +32, going down)
			v++;
		}
		else {
			v += 32;
		}
		break;
	case 0x4014: //OAMDMA
		primaryOAM[OAMADDR] = data;
		OAMADDR++;
		break;
	}
}

bool PPU::sendNMI() {
	bool hold = NMI;
	NMI = true;
	return hold;
}

void PPU::spriteEval() {
	for (int i = 0; i < 40; i++) {
		secondaryOAM[i] = 0xFF;
	}
	int spriteSize = (((PPUCTRL >> 5) & 0x01) == 1 ? 16 : 8);
	int n = 0;
	for (int i = 0; i < 64; i++) {
		int c_height = (tileY * 8 + Y) - primaryOAM[i * 4] - 1;
		if (c_height >= 0 && c_height < spriteSize) {
			secondaryOAM[n * 5] = i; //ID for which sprite in primary OAM it is
			secondaryOAM[n * 5 + 1] = primaryOAM[i * 4] + 1; //Y coor
			secondaryOAM[n * 5 + 2] = primaryOAM[i * 4 + 1]; //tile
			secondaryOAM[n * 5 + 3] = primaryOAM[i * 4 + 2]; //attributes
			secondaryOAM[n * 5 + 4] = primaryOAM[i * 4 + 3]; //X coor
			n++;
			if (n >= 8) {
				PPUSTATUS |= (1 << 5); //Sprite Overflow
				break;
			}
		}
	}
}

void PPU::getSpritePixel() {
	for (int i = 0; i < 8; i++) {
		sprX = (tileX * 8 + 7 - X) - secondaryOAM[i * 5 + 4];
		int spriteSize = (((PPUCTRL >> 5) & 0x01) == 1 ? 16 : 8);
		if (sprX >= 0 && sprX < 8) {
			c_index = secondaryOAM[i * 5 + 2];
			c_attri = secondaryOAM[i * 5 + 3];
			sprY = (tileY * 8 + Y) - secondaryOAM[i * 5 + 1];
			//TODO: FLIP SPRITE VERTICALLY, FIX LATER
			if ((c_attri >> 7) & 0x01) { //flip sprite vertically
				if (spriteSize == 8) {
					sprY = 7 - sprY;
				}
				else { //spriteSize == 16
					sprY = 15 - sprY;
				}
			}
			if (spriteSize == 8) {
				sprReg16_0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 3) & 0x01) + c_index * 16 + sprY);
				sprReg16_1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 3) & 0x01) + c_index * 16 + sprY + 8);
			}
			else {
				sprReg16_0 = m_unit.readCHR(0x1000 * (c_index & 0x01) + (c_index & 0xFE) * 16 + sprY);
				sprReg16_1 = m_unit.readCHR(0x1000 * (c_index & 0x01) + (c_index & 0xFE) * 16 + sprY + 8);
			}
			if ((c_attri >> 6) & 0x01) { //flip sprite horizontally
				sprPixel = ((sprReg16_0 >> sprX) & 0x01) | (((sprReg16_1 >> sprX) & 0x01) << 1);
			}
			else {
				sprPixel = ((sprReg16_0 >> (7 - sprX)) & 0x01) | (((sprReg16_1 >> (7 - sprX)) & 0x01) << 1);
			}
			if (sprPixel != 0x00) {
				sprPaletteNo = 16 + (c_attri & 0x03) * 4 + sprPixel;
				int spriteSize = (((PPUCTRL >> 5) & 0x01) == 1 ? 16 : 8);
				sprite0 = secondaryOAM[i * 5] == 0 ? true : false;
				break;
			}
		}
		else {
			sprPixel = 0x00;
		}
	}
}

uint8_t PPU::getPaletteColor(uint8_t paletteNo) {
	switch (paletteNo) {
	//universal background color: 0x00, other values are mirrors
	//anything not 0x00 can be coded to a different color but because of PPU hardware,
	//regardless of which case is given it will always map back to 0x00
	case 0x00: case 0x10:
	case 0x04: case 0x14:
	case 0x08: case 0x18:
	case 0x0C: case 0x1C:
		return paletteRAM[0];
	default:
		return paletteRAM[paletteNo];
	}
}

void PPU::incCX() {
	if ((v & 0x001F) == 31) {
		v &= ~0x001F;
		v ^= 0x0400;
	}
	else {
		v += 1;
	}
}

void PPU::incCY() {
	if ((v & 0x7000) != 0x7000) {
		v += 0x1000;
	}
	else {
		v &= ~0x7000;
		int y = (v & 0x03E0) >> 5;
		if (y == 29) {
			y = 0;
			v ^= 0x0800;
		}
		else if (y == 31) {
			y = 0;
		}
		else {
			y += 1;
		}
		v = (v & ~0x03E0) | (y << 5);
	}
}

void PPU::execute() {
	//break up v into components
	int fY = (v >> 12) & 0x7;
	int NT = (v >> 10) & 0x3;
	int cY = (v >> 5) & 0x1F;
	int cX = v & 0x1F;
	switch (renderState) {
	case 0:
		if (c_cycle == 1) {
			PPUSTATUS &= 0x1F; //clear vBlank, sprite 0 hit, and sprite overflow flags
		}
		else if (c_cycle == 256) {
			incCY();
		}
		else if (c_cycle == 257) {
			v = t;
		}
		else if (c_cycle >= 321 && c_cycle <= 336) {
			switch(c_cycle % 8){
			case 2:
				nt_byte = ciRAM[0x400 * ((v >> 10) & 0x3) + (v & 0x3FF)];
				break;
			case 4:
				//AT byte fetch
				break;
			case 6:
				bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + fY) << 8;
				break;
			case 0:
				bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + fY + 8) << 8;
				incCX();
				break;
			}
			bg_latch0 >>= 1;
			bg_latch1 >>= 1;
		}
		else if (c_cycle == 340) { //reached end of scanline
			renderState = 1;
			c_scanline = 0;
			c_cycle = -1;
		}
		break;
	case 1:
		if (c_cycle >= 1 && c_cycle <= 256) { //on screen
			bg_pixel = (sReg16_0 & 0x01) | ((sReg16_1 & 0x01) << 1);
			sReg16_0 >>= 1;
			sReg16_1 >>= 1;
			//this gives me palette number from AT for BG
			bgPaletteNo = ((ciRAM[(0x400 * (PPUCTRL & 0x03) + 0x3C0 + atY * 8 + atX) & 0x7FF] >> (((tileX % 4) >> 1) + ((tileY % 4) & 0x02)) * 2) & 0x03) * 4 + bg_pixel; //AT byte = ciRAM[0x3C0 + atY * 8 + atX]
			getSpritePixel();
			if (sprite0 && (bg_pixel != 0)) {
				PPUSTATUS |= 1 << 6; //sprite 0 hit flag
			}
			if (sprPixel && (((c_attri >> 5) & 0x01) == 0) && ((PPUMASK >> 4) & 0x01)) { //sprite pixel is visible AND sprite pixel has prio over background AND show sprites 
				screenBuffer[tileY * 2048 + Y * 256 + tileX * 8 + 7 - X] = colorPalette[getPaletteColor(sprPaletteNo)];
			}
			else if (((PPUMASK >> 3) & 0x01)) { //show background                                            
				screenBuffer[tileY * 2048 + Y * 256 + tileX * 8 + 7 - X] = colorPalette[getPaletteColor(bgPaletteNo)];
			}
			if (c_cycle >= 1 && c_cycle <= 248) {
				switch (c_cycle % 8) {
				case 2:
					nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX + 2];
					break;
				case 6:
					bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y);
					break;
				case 0:
					bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8);
					sReg16_0 |= bg_latch0 << 8;
					sReg16_1 |= bg_latch1 << 8;
					break;
				}
			}
		}
		else if (c_cycle >= 321 && c_cycle <= 336) {
			switch (c_cycle) {
			case 322:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX];
				break;
			case 326:
				bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y);
				break;
			case 328:
				bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8);
				break;
			case 330:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX + 1];
				break;
			case 334:
				bg_latch0 |= (m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y) << 8);
				break;
			case 336:
				bg_latch1 |= (m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8) << 8);
				sReg16_0 = bg_latch0;
				sReg16_1 = bg_latch1;
				break;
			}
		}
		else if (c_cycle == 340) { //scanline cleared, go next
			if (c_scanline == 239) { //render stage cleared, go post render
				renderState = 2; //post render
			}
			c_scanline++;
			spriteEval();
			c_cycle = -1;
		}
		break;
	}
}

//experimental solution
//BG rendering seems to work fine, yet to be hard tested though... AT fetching and decryption is not true to NES hardware implementation... YET
void PPU::execute2() {
	//break up v into components
	int fY = (v >> 12) & 0x7;
	int NT = (v >> 10) & 0x3;
	int cY = (v >> 5) & 0x1F;
	int cX = v & 0x1F;
	switch (renderState) {
	case 0: //pre-render  (-1 or 261)
		if (c_cycle == 1) {
			PPUSTATUS &= 0x1F; //clear vBlank, sprite 0 hit, and sprite overflow flags
		}
		else if (c_cycle >= 321 && c_cycle <= 336) {
			switch(c_cycle) {
			case 322:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX];
				break;
			case 326:
				bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y);
				break;
			case 328:
				bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8);
				break;
			case 330:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX + 1];
				break;
			case 334:
				bg_latch0 |= m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y) << 8;
				break;
			case 336:
				bg_latch1 |= m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8) << 8;
				sReg16_0 = bg_latch0;
				sReg16_1 = bg_latch1;
				break;
			}
		}
		else if (c_cycle == 340) { //reached end of scanline
			renderState = 1;
			c_scanline = 0;
			c_cycle = -1;
		}
		break;
	case 1: //render (0-239)
		if (c_cycle >= 1 && c_cycle <= 256) { //on screen
			bg_pixel = (sReg16_0 & 0x01) | ((sReg16_1 & 0x01) << 1);
			sReg16_0 >>= 1;
			sReg16_1 >>= 1;
			//this gives me palette number from AT for BG
			bgPaletteNo = ((ciRAM[(0x400 * (PPUCTRL & 0x03) + 0x3C0 + atY * 8 + atX) & 0x7FF] >> (((tileX % 4) >> 1) + ((tileY % 4) & 0x02)) * 2) & 0x03) * 4 + bg_pixel; //AT byte = ciRAM[0x3C0 + atY * 8 + atX]
			getSpritePixel();
			if (sprite0 && (bg_pixel != 0)) {
				PPUSTATUS |= 1 << 6; //sprite 0 hit flag
			}
			if (sprPixel && (((c_attri >> 5) & 0x01) == 0) && ((PPUMASK >> 4) & 0x01)) { //sprite pixel is visible AND sprite pixel has prio over background AND show sprites 
				screenBuffer[tileY * 2048 + Y * 256 + tileX * 8 + 7 - X] = colorPalette[getPaletteColor(sprPaletteNo)];
			}
			else if (((PPUMASK >> 3) & 0x01)) { //show background                                            
				screenBuffer[tileY * 2048 + Y * 256 + tileX * 8 + 7 - X] = colorPalette[getPaletteColor(bgPaletteNo)];
			}
			if (c_cycle >= 1 && c_cycle <= 248) {
				switch (c_cycle % 8) {
				case 2:
					nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX + 2];
					break;
				case 6:
					bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y);
					break;
				case 0:
					bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8);
					sReg16_0 |= bg_latch0 << 8;
					sReg16_1 |= bg_latch1 << 8;
					break;
				}
			}
			X++;
			if (X == 8) { //completed 1 8x8 tile row
				//reset x, goto next tile in row
				X = 0;
				tileX++;
				if (tileX % 4 == 0) {
					atX++;
				}
				if (tileX == 32) {
					atX = 0;
					tileX = 0;
					Y++;
					if (Y == 8) { //finished tile row
						Y = 0;
						tileY++;
						if (tileY % 4 == 0) {
							atY++;
						}
						if (tileY == 30) {
							atY = 0;
							tileY = 0;
						}
					}
				}
			}
		}
		else if (c_cycle >= 321 && c_cycle <= 336) {
			switch (c_cycle) {
			case 322:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY* 32 + tileX];
				break;
			case 326:
				bg_latch0 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y);
				break;
			case 328:
				bg_latch1 = m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8);
				break;
			case 330:
				nt_byte = ciRAM[0x400 * (PPUCTRL & 0x03) + tileY * 32 + tileX + 1];
				break;
			case 334:
				bg_latch0 |= (m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y) << 8);
				break;
			case 336:
				bg_latch1 |= (m_unit.readCHR(0x1000 * ((PPUCTRL >> 4) & 0x01) + nt_byte * 16 + Y + 8) << 8);
				sReg16_0 = bg_latch0;
				sReg16_1 = bg_latch1;
				break;
			}
		}
		else if (c_cycle == 340) { //scanline cleared, go next
			if (c_scanline == 239) { //render stage cleared, go post render
				renderState = 2; //post render
			}
			c_scanline++;
			spriteEval();
			c_cycle = -1;
		}
		break;
	case 2: //post-render (240-260)
		//dummy scanline: do nothing
		if (c_scanline == 241 && c_cycle == 1) {
			PPUSTATUS |= (1 << 7); //vBlank flag set
			if (PPUCTRL >> 7) { //generate NMI at start of vBlank interval (0: off; 1: on)
				NMI = false;
			}
		}
		if (c_cycle == 340) {
			if (c_scanline == 260) {
				renderState = 0;
			}
			c_scanline++;
			c_cycle = -1;
		}
		break;
	}
	c_cycle++;
};

int* PPU::drawScreen() {
	return screenBuffer;
}

void PPU::drawPatternTable() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Pattern Table", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 1024, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 256);
	SDL_Event e;

	int buffer[32768];
	uint16_t eightPixels0, eightPixels1;

	//this loop can be simplified but is left as is as it gives me the best way to visualize how the pattern table is drawn
	for (int tileRow = 0; tileRow < 32; tileRow++) {
		//this loop will keep track of which tileRow I am
		for (int row = 0; row < 8; row++) {
			//this loop will keep track of which row in the tileRow I am
			for (int tileNo = 0; tileNo < 16; tileNo++) {
				//this loop will keep track of which tile I am
				eightPixels0 = m_unit.readCHR(tileRow*256 + row + tileNo*16); //fetches 1 line of tile data
				eightPixels1 = m_unit.readCHR(tileRow*256 + row + tileNo*16 + 8); //fetches 1 mirrored line of tile data 
				for (int pixel = 7; pixel >= 0; pixel--) {
					//this loop will keep track of which pixel in the tile I am
					int temp = ((eightPixels0 >> pixel) & 0x01) | (((eightPixels1 >> pixel) & 0x01) << 1);
					switch (temp) {
					case 3:
						buffer[tileRow*1024 + row*128 + tileNo*8 + 7-pixel] = 0xFFFFFFFF;
						break;
					case 2:
						buffer[tileRow*1024 + row*128 + tileNo*8 + 7-pixel] = 0xFFB0B0B0;
						break;
					case 1:
						buffer[tileRow*1024 + row*128 + tileNo*8 + 7-pixel] = 0xFF606060;
						break;
					default:
						buffer[tileRow*1024 + row*128 + tileNo*8 + 7-pixel] = 0;
						break;
					}
				}
			}
		}
	}
	while (true) {
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_SCANCODE_ESCAPE) {
				return;
			}
		}
		SDL_UpdateTexture(texture, NULL, buffer, 512);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
}

void PPU::drawNameTable() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Pattern Table", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	SDL_Event e;

	int buffer[61440];
	uint16_t eightPixels0, eightPixels1;

	//this loop can be simplified but is left as is as it gives me the best way to visualize how the pattern table is drawn
	for (int tileRow = 0; tileRow < 30; tileRow++) {
		//this loop will keep track of which tileRow I am
		for (int row = 0; row < 8; row++) {
			//this loop will keep track of which row in the tileRow I am
			for (int tileNo = 0; tileNo < 32; tileNo++) {
				//this loop will keep track of which tile I am
				eightPixels0 = m_unit.readCHR(ciRAM[tileRow * 32 + tileNo] * 16 + row); //fetches 1 line of tile data
				eightPixels1 = m_unit.readCHR(ciRAM[tileRow * 32 + tileNo] * 16 + row + 8); //fetches 1 mirrored line of tile data 
				for (int pixel = 7; pixel >= 0; pixel--) {
					//this loop will keep track of which pixel in the tile I am
					int temp = ((eightPixels0 >> pixel) & 0x01) | (((eightPixels1 >> pixel) & 0x01) << 1);
					switch (temp) {
					case 3:
						buffer[tileRow * 2048 + row * 256 + tileNo * 8 + 7 - pixel] = 0xFFFFFFFF;
						break;
					case 2:
						buffer[tileRow * 2048 + row * 256 + tileNo * 8 + 7 - pixel] = 0xFFB0B0B0;
						break;
					case 1:
						buffer[tileRow * 2048 + row * 256 + tileNo * 8 + 7 - pixel] = 0xFF606060;
						break;
					default:
						buffer[tileRow * 2048 + row * 256 + tileNo * 8 + 7 - pixel] = 0;
						break;
					}
				}
			}
		}
	}
	while (true) {
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_SCANCODE_ESCAPE) {
				return;
			}
		}
		SDL_UpdateTexture(texture, NULL, buffer, 1024);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
}