#pragma once
#include <string>
#include <list>

class Regex {
public:
	virtual int match(char const *) = 0;
};

class CharClass : public Regex {
public:
	std::string contents;
	CharClass(std::string c);

	int match(char const *text) {
		return contents.find(*text) == std::string::npos ? -1 : 1;
	}
};

class Star : public Regex {
public:
	CharClass* operand;
	Star(CharClass* operand);
	int match(char const *text) {
		int n, consumed = 0;
		while ((n = operand->match(text)) > 0)
		{
			consumed += n;
			text += n;
		}
		return consumed;
	}
};

class Seq : public Regex {
public:
	std::list<Regex*> cells;
	Seq(std::list<Regex*> cells);
	int match(char const *text) {
		int chars, consumed = 0;
		for (auto c:cells) {
			if ((chars = c->match(text)) < 0)  return -1;
			consumed += chars;
			text += chars;
		}
		return consumed;
	}
};