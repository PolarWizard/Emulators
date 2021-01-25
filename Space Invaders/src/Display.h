#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream>

class display : public computer {
public:
	void p() {
		std::cout << "display";
	}
};

#endif 