#pragma once
#include "regex.h"
#include "tree.h"

class Parser {
public:
	const char* input;
	Parser(const char* input) : input(input) {}
	Tree* tree;
	Tree* currentChild;

	//RULES
	bool TERM(const char *lit, Tree** result) {
		const char* start = input;
		int i;
		for (i = 0; lit[i] != 0; i++)
			if (input[i] != lit[i])
				return false;
		input += i;
		*result = new Tree("STRING", (char*)start, input - start);
		return true;
	}

	bool HEX(Tree** result) {
		const char* start = input;
		CharClass nonzerohex("123456789ABCDEFabcdef");
		CharClass hexdigit("0123456789ABCDEFabcdef");
		CharClass hexstart1("0");
		CharClass hexstart2("x");
		Seq hextest({ &hexstart1,&hexstart2, new Star(&hexdigit) });

		int r = hextest.match(input);
		if (r != -1) {
			input += r;
			*result = new Tree("HEX", (char*)start, input - start);
			return true;
		}
		input = start;
		return false;
	}

	bool NUM(Tree** result) {
		Tree* child1;
		const char* start = input;
		CharClass nonzero("123456789");
		CharClass digit("0123456789");
		Seq numtest({ new Star(&digit) });
		const char* beforesign = input;
		if (TERM("-", &child1) || TERM("+", &child1) || TERM("", &child1)) {
			int r = numtest.match(input);
			if (r != -1) {
				input += r;
				*result = new Tree("INTEGER", (char*)start, beforesign - start);
				return true;
			}
		}
		input = start;
		return false;
	}

	bool DEC(Tree** result) {
		const char* start = input;
		CharClass nonzero("123456789");
		CharClass digit("0123456789");
		CharClass dot(".");
		Tree* child1;

		Seq dectest({&nonzero, new Star(&digit), &dot, new Star(&digit)});

		const char* beforesign = input;
		if (TERM("-", &child1) || TERM("+", &child1) || TERM("", &child1)) {
			int r = dectest.match(input);
			if (r != -1) {
				input += r;
				*result = new Tree("DECIMAL", (char*)start, beforesign - start);
				return true;
			}
		}
		
		input = start;
		return false;
	}

	bool IDENT(Tree** result) {
		const char* start = input;
		CharClass equal("=");
		CharClass character("[]\"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstvwxyz");
		Seq identtest({ new Star(&character),&equal });
		int r = identtest.match(input);
		if (r != -1) {
			input += r;
			*result = new Tree("IDENTIFIER", (char*)start, input - start - 1);
			return true;
		}
		return false;
	}

	bool STRING(Tree** result) {
		const char* start = input;
		CharClass quote("\"");
		CharClass character("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ");
		Seq stringtest({ &quote, new Star(&character), &quote });
		int r = stringtest.match(input);
		if (r != -1) {
			input += r;
			*result = new Tree("STRING", (char*)start+1, input - start -2);
			return true;
		}
		input = start;
		return false;
	}

	bool NEWLINE(Tree** result) {
		Tree* child1;
		const char* start = input;
		if (TERM("\n", &child1)) {
			*result = new Tree("NEWLINE", (char*)start, input - start);
			return true;
		}
		input = start;
		return false;
	}

	bool TABSPACE() {
		Tree* child1;
		const char* start = input;
		if (TERM("\n", &child1) || TERM("\t", &child1) || TERM(" ", &child1)) {
			return TABSPACE();
		}
		input = start;
		return true;
	}

	bool DECNUM(Tree** result) {
		const char* start = input;
		Tree* child1;
		if (DEC(&child1) ||NUM(&child1)) {
			*result = new Tree("NUMBER", (char*)start, input - start);
			//(*result)->children.push_back(child1);
			return true;
		}
		input = start;
		return false;
	}

	bool VERT(Tree** result) { //VERT: TERM("(") (VERT3 || VERT5 VERT) || EMPTY
		Tree* child1;
		Tree* childnum1;
		Tree* childnum2;
		Tree* childnum3;
		Tree* childnum4;
		Tree* childnum5;
		Tree* childnext;

		const char* start = input;
		const char* start2;
		TABSPACE();
		if (TERM("(", &child1)) {
			TABSPACE();
			//VERT3: DEC TERM(",") DEC TERM(",") DEC TERM(")") TERM(",") || EMPTY
			start2 = input;
			if (DECNUM(&childnum1) && TERM(",", &child1) && TABSPACE() && DECNUM(&childnum2) && TERM(",", &child1) && TABSPACE() && DECNUM(&childnum3) && TERM(")", &child1) && (TERM(",", &child1) || TERM("", &child1))) {
				*result = new Tree("VERT3", (char*)start, input - start);
				(*result)->children.push_back(childnum1);
				(*result)->children.push_back(childnum2);
				(*result)->children.push_back(childnum3);
				if (VERT(&childnext)) {
					(*result)->children.push_back(childnext);
					return true;
				}
				return true;
			} 
			else {
				input = start2;
				TABSPACE();
				//VERT3: DEC TERM(",") DEC TERM(",") DEC TERM(",") DEC TERM(",") DEC TERM(")") TERM(",") || EMTPY
				if (DECNUM(&childnum1) && TERM(",", &child1) && TABSPACE() && DECNUM(&childnum2) && TERM(",", &child1) && TABSPACE() && DECNUM(&childnum3)
					&& TERM(",", &child1) && TABSPACE() && DECNUM(&childnum4) && TERM(",", &child1) && TABSPACE() && DECNUM(&childnum5) && TERM(")", &child1) && (TERM(",", &child1) || TERM("", &child1))) {
					*result = new Tree("VERT5", (char*)start, input - start);
					(*result)->children.push_back(childnum1);
					(*result)->children.push_back(childnum2);
					(*result)->children.push_back(childnum3);
					(*result)->children.push_back(childnum4);
					(*result)->children.push_back(childnum5);
					if (VERT(&childnext)) {
						(*result)->children.push_back(childnext);
						return true;
					}
					return true;
				}
				return true;
			}
		}
		input = start;
		return false;
	}

	bool MESH(Tree** result) { //MESH: TERM("Mesh(") STRING TERM(")") NEWLINE TERM("{") NEWLINE VERT || NEWLINE TERM("}")
		const char* start = input;
		Tree* child1; 
		Tree* child2;
		Tree* child3;
		TABSPACE();
		if (TERM("Mesh(", &child1) && STRING(&child2) && TERM(")", &child1) && TABSPACE() && TERM("{", &child1) && TABSPACE() &&
			(VERT(&child3)  && TABSPACE() && TERM("}", &child1))) {
			*result = new Tree("MESH", (char*)start, input - start);
			(*result)->children.push_back(child2);
			(*result)->children.push_back(child3);
			return true;
		}
		input = start;
		return false;
	}

	bool VECTOR(Tree** result) {
		const char* start = input;
		Tree* child1;
		Tree* child2;
		Tree* child3;
		Tree* child4;
		Tree* childnext;
		TABSPACE();
		if (TERM("(", &child1) && DECNUM(&child2) && TERM(",", &child1) && TABSPACE() && DECNUM(&child3) && TERM(",", &child1) && DECNUM(&child4) && TERM(")", &child1)) {
			*result = new Tree("VECTOR", (char*)start, input - start);
			(*result)->children.push_back(child2);
			(*result)->children.push_back(child3);
			(*result)->children.push_back(child4);
			if (TERM(",", &child1) && VECTOR(&childnext)) {
				(*result)->children.push_back(childnext);
				return true;
			}
			return true;
		}
		input = start;
		return false;
	}

	bool TEXTURE(Tree** result) {
		const char* start = input;
		Tree* child1;
		Tree* child2;
		Tree* child3;
		TABSPACE();
		if (TERM("Texture(", &child1) && STRING(&child2) && TERM(")", &child1) && TABSPACE() && TERM("{", &child1) && VECTOR(&child3) && TABSPACE() && TERM("}", &child1) ) {
			*result = new Tree("TEXTURE", (char*)start, input - start);
			(*result)->children.push_back(child2);
			(*result)->children.push_back(child3);
			return true;
		}
		input = start;
		return false;
	}

	bool ASSIGN(Tree** result) {
		Tree* child1; Tree* child2; 
		const char* start = input;
		if (IDENT(&child1)) {
			if (STRING(&child2) || DEC(&child2) || NUM(&child2) || TABLE(&child2) || HEX(&child2)) {
				*result = new Tree("ASSIGN", (char*)start, input - start);
				(*result)->children.push_back(child1);
				(*result)->children.push_back(child2);
				return true;
			}
		}
		input = start;
		return false;
	}

	bool TABLE(Tree** result) {
		Tree* child1;
		Tree* child2;
		Tree* child3;
		const char* start = input;
		if (TERM("{",&child1)) {
			if (TABLE2(&child2)) {
				if (TERM("}",&child3)) {
					*result = new Tree("TABLE", (char*)start, input - start);
					(*result)->children.push_back(child1);
					(*result)->children.push_back(child2);
					(*result)->children.push_back(child3);
					return true;
				}
			}
		}
		input = start;
		return false;
	}

	bool FIELDSEP(Tree** result) {
		Tree* child1;
		const char* start = input;
		if (TERM(",", &child1) || TERM(";", &child1)) {
			*result = new Tree("FIELDSEP", (char*)start, input - start);
			(*result)->children.push_back(child1);
			return true;
		}
		input = start;
		return false;
	}

	bool MESHADD(Tree** result) {
		Tree* child1;
		Tree* child2;
		const char* start = input;
		if (TERM("Mesh(", &child1) && STRING(&child2) && TERM(")", &child1)) {
			*result = new Tree("MESHADD", (char*)start, input - start);
			(*result)->children.push_back(child2); //add mesh name as child
			return true;
		}
		input = start;
		return false;
	}

	bool BIND(Tree** result) {
		Tree* child1;
		Tree* child2;
		Tree* child3;
		const char* start = input;
		if (TERM("Bind(", &child1) && STRING(&child2) && TERM(",", &child1) && TABSPACE() && MESHADD(&child3) && TERM(")", &child1)) {
			*result = new Tree("BIND", (char*)start, input - start);
			(*result)->children.push_back(child2); //name
			(*result)->children.push_back(child3); //mesh
			return true;
		}
		input = start;
		return false;
	}

	bool SCENEACTION(Tree** result) {
		Tree* child1;
		Tree* child2;
		const char* start = input;
		TABSPACE();
		if (MESHADD(&child1) || BIND(&child1)) {
			*result = new Tree("SCENEACTION", (char*)start, input - start);
			(*result)->children.push_back(child1);
			if (SCENEACTION(&child2)) {
				(*result)->children.push_back(child2); //next sceneAction node
				return true;
			}
			return true;
		}
		input = start;
		return false;
	}

	bool SCENE(Tree** result) {
		Tree* child1;
		Tree* child2;
		const char* start = input;
		if (TERM("Scene()",&child1) && TABSPACE() && TERM("{", &child1)) {
			TABSPACE();
			if (SCENEACTION(&child2) && TABSPACE() && TERM("}", &child1)) {
				*result = new Tree("SCENE", (char*)start, input - start);
				(*result)->children.push_back(child2);
				return true;
			}
		}
		input = start;
		return false;
	}

	bool DESCRIPTOR(Tree** result) { //DESCRIPTOR: MESH || TEXTURE || SCENE DESCRIPTOR
		Tree* child1;
		Tree* child2;
		const char* start = input;
		TABSPACE();
		if (MESH(&child1) || TEXTURE(&child1) || SCENE(&child1)) {
			*result = new Tree("DESCRIPTOR", (char*)start, input - start);
			(*result)->children.push_back(child1);
			if (DESCRIPTOR(&child2)) {
				(*result)->children.push_back(child2);
			}
			return true;
		}
		input = start;
		return false;
	}

	bool LOOP(Tree** result) {
		Tree* child1;
		Tree* child2;
		Tree* child3;
		const char* start = input;
		if (HEX(&child1) || TABLE(&child1) || NUM(&child1) || DEC(&child1) || ASSIGN(&child1) || STRING(&child1)) {
			*result = new Tree("FIELD", (char*)start, input - start);
			(*result)->children.push_back(child1);
			if (FIELDSEP(&child2)) {
				(*result)->children.push_back(child2);
				if (TABLE2(&child3)) {
					(*result)->children.push_back(child3);
					return true;
				}
				return true;
			}
			return true;
		}
		*result = new Tree("EMPTY FIELD", (char*)start, input - start);
		return true;
	}

	bool TABLE2(Tree** result) {
		Tree* child1;
		const char* start = input;
		if (LOOP(&child1)) {
			*result = new Tree("FIELDLIST", (char*)start, input - start);
			(*result)->children.push_back(child1);
			return true;
		}
		return false;
	}
};