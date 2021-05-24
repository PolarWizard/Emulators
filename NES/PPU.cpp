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

uint8_t PPU::readIO(uint16_t addr) {
	uint8_t data = 0;
	switch (addr) {
	case 0x2002: //PPUSTATUS
		w = false;
		data = PPUSTATUS;
		PPUSTATUS &= 0x7F;
		break;
	case 0x2004: //OAMDATA
		data = primaryOAM[OAMADDR];
		break;
	case 0x2007: //PPUDATA
		data = PPUlatch;
		PPUlatch = read(v);
		if (v >= 0x3F00) {
			data = PPUlatch;
		} 
		v += (((PPUCTRL >> 2) & 0x01) ? 32 : 1); //VRAM addr inc per CPU R/W of PPUDATA (0: +1, going across; 1: +32, going down)
		break;
	}
	return data;
}

void PPU::writeIO(uint16_t addr, uint8_t data) {
	switch (addr) {
	case 0x2000: //PPUCTRL
		PPUCTRL = data;
		t &= 0x73FF; //0111 0011 1111 1111
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
			t &= 0x7FE0; //0111 1111 1110 0000
			t |= (data >> 3); //0111 1111 111X XXXX
			x = data & 0x07;
			w = true;
		}
		else {
			t &= 0x0C1F; //0000 1100 0001 1111
			t |= ((data >> 3) << 5);
			t |= ((data & 0x07) << 12); //0yyy 11YY YYY1 1111
			w = false;
		}
		break;
	case 0x2006: //PPUADDR
		if (!w) {
			t &= 0x00FF;
			t |= ((data & 0x3F) << 8);
			w = true;
		}
		else {
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
			//DO LATER: COME UP WITH BETTER MIRRORING SCHEME
			int addr = v & 0x2FFF;
			if (m_unit.mirroring() == 0) { //horizantal
				if (addr >= 0x2400 && addr < 0x2800) {
					addr -= 0x400;
				}
				else if (addr >= 0x2800 && addr < 0x2c00) {
					addr -= 0x400;
				}
				else if (addr >= 0x2c00 && addr < 0x3000) {
					addr -= 0x800;
				}
			}
			else { //vertical
				if (addr >= 0x2800 && addr < 0x3000) {
					addr -= 0x800;
				}
			}
			ciRAM[addr & 0x7FF] = data;
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
		v += (((PPUCTRL >> 2) & 0x01) ? 32 : 1);//VRAM addr inc per CPU R/W of PPUDATA (0: +1, going across; 1: +32, going down)
		break;
	case 0x4014: //OAMDMA
		primaryOAM[OAMADDR] = data;
		OAMADDR++;
		break;
	}
}

void PPU::write(uint16_t addr, uint8_t data) {

}

uint8_t PPU::read(uint16_t addr) {
	if (addr < 0x2000) { //pattern table:: PT0: 0x0000 - 0x0FFF, PT1: 0x1000 - 0x1FFF
		return m_unit.readCHR(addr);
	}
	else if (addr < 0x3F00) { //nametable: NT0: 0x2000-0x23FF, NT1: 0x2400-0x27FF, NT2: 0x2800-0x2BFF, NT3: 0x2C00-0x2FFF, 0x3000-0x3EFF Mirrors of 0x2000-0x2EFF  
		return ciRAM[addr & 0x07FF];
	}
	else if (addr < 0x4000) { //Palette RAM indexes:: 0x3F00-0x3F1F, 0x3F20-0x3FFF Mirrors of 0x3F00-0x3F1F
		return getPaletteColor(addr & 0x1F);
	}

	return 0;
}

bool PPU::sendNMI() {
	bool hold = NMI;
	NMI = true;
	return hold;
}

void PPU::spriteEval2() {
	for (int i = 0; i < 40; i++) {
		secondaryOAM[i] = 0xFF;
	}
	int spriteSize = (((PPUCTRL >> 5) & 0x01) == 1 ? 16 : 8);
	int n = 0;
	for (int i = 0; i < 64; i++) {
		int c_height = scanline - primaryOAM[i * 4];
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

void PPU::getSpritePixel2() {
	for (int i = 0; i < 8; i++) {
		sprX = (((v & 0x1F) << 3) - 0x10) + (pixCounter % 8)  - secondaryOAM[i * 5 + 4];
		int spriteSize = (((PPUCTRL >> 5) & 0x01) == 1 ? 16 : 8);
		if (sprX >= 0 && sprX < 8) {
			c_index = secondaryOAM[i * 5 + 2];
			c_attri = secondaryOAM[i * 5 + 3];
			sprY = scanline - secondaryOAM[i * 5 + 1];
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
				sprReg16_0 = read(c_index * 16 + sprY);
				sprReg16_1 = read(c_index * 16 + sprY + 8);
			}
			else {
				sprReg16_0 = read(0x1000 * (c_index & 0x01) + (c_index & 0xFE) * 16 + sprY);
				sprReg16_1 = read(0x1000 * (c_index & 0x01) + (c_index & 0xFE) * 16 + sprY + 8);
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
	//if fine Y < 7
	if ((v & 0x7000) != 0x7000) { 
		v += 0x1000;
	}
	else {
		v &= ~0x7000; //reset fine Y to 0
		int y = (v & 0x03E0) >> 5; //set y == coarse Y
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
	tick();
	//background and pixel render
	//v address 
	//0yyy-NNYY-YYYX-XXXX
	switch (renderState) {
	case 0: //pre-render (-1 or 261)
		switch (cycle) {
		case 1:
			PPUSTATUS &= 0x1F; //clear vBlank, sprite 0 hit, and sprite overflow flags
			break;
		case 340:
			renderState = 1;
			break;
		default:
			if (cycle == 257) {
				if ((PPUMASK & 0x1E) == 0x1E) {
					v = (v & ~0x041F) | (t & 0x041F); //hori(v) = hori(t) 
				}
			}
			else if (cycle >= 280 && cycle <= 304) {
				if ((PPUMASK & 0x1E) == 0x1E) {
					v = (v & ~0x7BE0) | (t & 0x7BE0); //vert(v) = vert(t)
				}
			}
			else if (cycle >= 321 && cycle <= 336) {
				backgroundOperations();
			}
			break;
		}
		break;
	case 1: //visible frames 0-239
		switch (cycle) {
		case 340: //scanline finished
			if (scanline == 239) { //render stage cleared, go post render
				renderState = 2; //post render
				pixCounter = 0;
			}
			break;
		default:
			if (cycle >= 1 && cycle <= 256) { //on screen
				if (cycle == 256) {
					if ((PPUMASK & 0x1E) == 0x1E) {
						incCY();
					}
				}
				bg_pixel = ((bgReg16_0 >> (15 - x)) & 0x1) | (((bgReg16_1 >> (15 - x)) & 0x1) << 1);
				at_pixel = ((atReg16_0 >> (15 - x)) & 0x1) | (((atReg16_1 >> (15 - x)) & 0x1) << 1);
				uint8_t colorSelect = ((at_pixel << 2) | bg_pixel);
				getSpritePixel2();
				if ((PPUMASK & 0x1E) == 0x1E) {
					if (sprite0 && (bg_pixel != 0)) {
						PPUSTATUS |= 1 << 6; //sprite 0 hit flag
					}
					if (sprPixel && (((c_attri >> 5) & 0x01) == 0)) { //sprite pixel is visible AND sprite pixel has prio over background
						screenBuffer[pixCounter++] = colorPalette[read(0x3F00 | sprPaletteNo)];
					}
					else {
						screenBuffer[pixCounter++] = colorPalette[read(0x3F00 | colorSelect)];
					}
				}
				else {
					screenBuffer[pixCounter++] = 0;
				}
				backgroundOperations();
			}
			else if (cycle == 257) {
				if ((PPUMASK & 0x1E) == 0x1E) {
					v = (v & ~0x041F) | (t & 0x041F); //hori(v) = hori(t) 
				}
				spriteEval2();
			}
			else if (cycle >= 321 && cycle <= 336) {
				backgroundOperations();
			}
			break;
		}
		break;
	case 2: //post-render (240-260)
	//dummy scanline: do nothing
		if (scanline == 241 && cycle == 1) {
			PPUSTATUS |= (1 << 7); //vBlank flag set
			if (PPUCTRL >> 7) { //generate NMI at start of vBlank interval (0: off; 1: on)
				NMI = false;
			}
		}
		if (cycle == 340) {
			if (scanline == 260) {
				renderState = 0;
			}
		}
		break;
	}
	
}

void PPU::tick() {
	cycle++;
	if (cycle > 340) {
		cycle = 0;
		scanline++;
		if (scanline > 261) scanline = 0;
	}
}

void PPU::backgroundOperations() {
	bgReg16_0 <<= 1;
	bgReg16_1 <<= 1;
	atReg16_0 <<= 1;
	atReg16_1 <<= 1;
	switch (cycle % 8) {
	case 1:
		nt_byte = read(0x2000 | (v & 0x03FF));
		break;
	case 3:
		at_byte = read(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x7));
		break;
	case 5:
		bgLatch0 = read(0x1000 * ((PPUCTRL >> 4) & 0x01) | (nt_byte << 4) | ((v >> 12) & 0x7));
		break;
	case 7:
		bgLatch1 = read(0x1000 * ((PPUCTRL >> 4) & 0x01) | (nt_byte << 4) | ((v >> 12) & 0x7) | 0x8);
		break;
	case 0:
		BG_Registers_Reload();
		if ((PPUMASK & 0x1E) == 0x1E) {
			incCX();
		}
		break;
	}
}

void PPU::vAddrUpdate() {
	if (scanline < 240 || scanline == 261) {
		if ((cycle % 8) == 0) {
			if (cycle != 256) {
				incCX();
			}
			else {
				incCY();
			}
		}
		else if (cycle == 257) {
			v = (v & ~0x041F) | (t & 0x041F);
		}
		else if (cycle >= 280 && cycle <= 304) {
			v = (v & ~0x7BE0) | (t & 0x7BE0);
		}
		
	}
}

void PPU::render() {
	//background and pixel render
	//vram address 
	//0yyy-NNYY-YYYX-XXXX
	switch (renderState) {
	case 0: //pre-render (-1 or 261)
		preRender();
		break;
	case 1: //visible frames 0-239
		visibleRender();
		break;
	case 2: //post-render (240-260)
	//dummy scanlines: do nothing
		postRender();
		break;
	}
}

void PPU::preRender() {
	switch (cycle) {
	case 1:
		PPUSTATUS &= 0x1F; //clear vBlank, sprite 0 hit, and sprite overflow flags
		break;
	case 340:
		renderState = 1;
		break;
	default:
		if (cycle >= 321 && cycle <= 336) {
			backgroundOperations();
		}
		break;
	}
}

void PPU::visibleRender() {
	switch (cycle) {
	case 340: //scanline finished
		if (scanline == 239) { //render stage cleared, go post render
			renderState = 2; //post render
			pixCounter = 0;
		}
		break;
	default:
		if (cycle >= 1 && cycle <= 256) { //on screen
			backgroundOperations();
		}
		else if (cycle == 257) {
			spriteEval2();
		}
		else if (cycle >= 321 && cycle <= 336) {
			backgroundOperations();
		}
		break;
	}
}

void PPU::postRender() {
	if (scanline == 241 && cycle == 1) {
		PPUSTATUS |= (1 << 7); //vBlank flag set
		if (PPUCTRL >> 7) { //generate NMI at start of vBlank interval (0: off; 1: on)
			NMI = false;
		}
	}
	if (cycle == 340) {
		if (scanline == 260) {
			renderState = 0;
		}
	}
}

int* PPU::drawScreen() {
	return screenBuffer;
}

void PPU::BG_Registers_Reload() {
	bgReg16_0 |= bgLatch0;
	bgReg16_1 |= bgLatch1;
	int coarseX = ((v >> 1) & 0x1); //XXXGX -> grab G from v
	int coarseY = ((v >> 6) & 0x1); //YYYGY -> grab G from v
	at_4bits = ((at_4bits << 2) & 0xF) | ((at_byte >> ((coarseY << 2) | (coarseX << 1))) & 0x3); //atByte >> YX0 then << 2, where Y and X is either 0 or 1
	//CHANGE LATER:
	for (int i = 0; i < 8; i++) {
		atReg16_0 |= ((at_4bits & 0x1) << i);
		atReg16_1 |= (((at_4bits >> 1) & 0x1) << i);
	}
}
