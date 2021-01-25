#include <iostream>
#include <fstream>
#include <iomanip>
#include "Intel 8080.h"

using namespace std;

int byte;
unsigned char* element;
unsigned int* buffer;
unsigned int* finalBuffer;

//invaders.concatenated 
//cpudiag.bin
//8080EXM
//TST8080

void Intel_8080::loadROM() {
	ifstream myfile;
	myfile.open("C:/Users/domin/OneDrive/Desktop/TST8080.COM", ios::binary | ios::ate);
	if (!myfile.is_open()) {
		cout << "failed to open";
		return;
	}
	byte = myfile.tellg();
	myfile.seekg(0, ios::beg);
	element = new unsigned char[byte];
	myfile.read((char*)element, byte);
	buffer = new unsigned int[byte];
	for (int i = 0; i < byte; ++i) {
		buffer[i] = (unsigned int)element[i];
	}
	for (int i = 0; i < byte; ++i) {
		RAM[0x100+i] = buffer[i];
	}
}

void Intel_8080::viewMemoryContents() {
	for (int i = 0; i < byte; i++) {
		cout << hex << setw(2) << setfill('0') << (int)RAM[i] << endl;
	}
}

int main() {
	int a = 0;
	Intel_8080 i8080;
	i8080.init();
	i8080.loadROM();
	//i8080.viewMemoryContents();
	while (1) {
		a = i8080.execute(&i8080);
		if (a == 1) {
			return 0;
		}
	}
	return 0;
}
