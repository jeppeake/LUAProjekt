#include "regex.h"

CharClass::CharClass(std::string c) :
	contents(c) {}

Star::Star(CharClass* operand) : 
	operand(operand) {}

Seq::Seq(std::list<Regex*> cells) : 
	cells(cells) {}