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

	bool NUM(Tree** result) {  //NUM: "-" | "+" | EMPTY DIGIT*
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

	bool DEC(Tree** result) { //DEC: "-" | "+" | EMPTY [1.9] [0-9]* "." [0-9]*
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

	bool STRING(Tree** result) {  //STRING: '"' [A-z0-9]* '"'
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

	bool NEWLINE(Tree** result) { //NEWLINE: "\n"
		Tree* child1;
		const char* start = input;
		if (TERM("\n", &child1)) {
			*result = new Tree("NEWLINE", (char*)start, input - start);
			return true;
		}
		input = start;
		return false;
	}

	bool TABSPACE() { //["\n","\t"," "]* TABSPACE | EMPTY
		Tree* child1;
		const char* start = input;
		if (TERM("\n", &child1) || TERM("\t", &child1) || TERM(" ", &child1)) {
			return TABSPACE();
		}
		input = start;
		return true;
	}

	bool DECNUM(Tree** result) { //DECNUM: DEC | NUM
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

	bool VERT(Tree** result) { //VERT: TERM("(") (VERT3 | VERT5 VERT) | EMPTY
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
			//VERT3: DECNUM TERM(",") DECNUM TERM(",") DECNUM TERM(")") TERM(",") | EMPTY
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
				//VERT3: DECNUM TERM(",") DECNUM TERM(",") DECNUM TERM(",") DECNUM TERM(",") DECNUM TERM(")") TERM(",") | EMTPY
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

	bool MESH(Tree** result) { //MESH: TERM("Mesh(") STRING TERM(")") NEWLINE TERM("{") VERT | TERM("}")
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

	bool VECTOR(Tree** result) { //VECTOR: "(" DECNUM "," DECUM "," DECNUM ")" VECTOR | EMPTY
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

	bool TEXTURE(Tree** result) { //TEXTURE: "Texture(" STRING ")" "{" VECTOR "}" 
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

	bool MESHADD(Tree** result) { //MESHADD: "Mesh(" STRING ")"
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

	bool BIND(Tree** result) { //BIND: "Bind(" STRING "," MESHADD ")"
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

	bool SCENEACTION(Tree** result) { //SCENEACTION: MESHADD | BIND SCENACTION | EMPTY
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

	bool SCENE(Tree** result) { //SCENE: "Scene()" "{" SCENEACTION "}"
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

	bool DESCRIPTOR(Tree** result) { //DESCRIPTOR: MESH | TEXTURE | SCENE DESCRIPTOR
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
};