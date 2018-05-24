#pragma once
#include <string>
#include <list>
#include <iostream>

#include <irrlicht.h>
#include <vector>
#include <string>

using namespace irr::video;
using namespace irr::scene;

struct loadedmesh {
	irr::scene::SMesh* mesh;
	std::string name;
	loadedmesh(irr::scene::SMesh* mesh, std::string name) : mesh(mesh), name(name) {}
};

class Tree {
public:
	std::string       lexeme, tag;
	std::vector<Tree*> children;
	Tree(std::string t, char *l, int size) : tag(t), lexeme(l, size) {}

	

	//std::vector<loadedmesh> meshes;



	void dump(int depth = 0) {
		for (int i = 0; i<depth; i++)
			std::cout << " "; 
		// Recurse over the children 
		std::cout << tag << ": " << lexeme << "\n";
		for (auto child : children) {
			child->dump(depth+1);
		}
	}

	void cumulateVertices(std::vector<irr::video::S3DVertex>* vertices) {
		if (tag.compare("VERT3") == 0) { //vertex without uv coords
			float x = std::stof(children.at(0)->lexeme);
			float y = std::stof(children.at(1)->lexeme);
			float z = std::stof(children.at(2)->lexeme);
			vertices->push_back(irr::video::S3DVertex(x, y, z, 0, 1, 0, irr::video::SColor(255, 0, 255, 255), 0, 1));
			if (children.size() == 4) {
				//step down the vertex list
				children.at(3)->cumulateVertices(vertices);
			}
		} 
		else if (tag.compare("VERT5") == 0) { //Vertex with uv coords
			float x = std::stof(children.at(0)->lexeme);
			float y = std::stof(children.at(1)->lexeme);
			float z = std::stof(children.at(2)->lexeme);
			float tu = std::stof(children.at(3)->lexeme);
			float tv = std::stof(children.at(4)->lexeme);
			vertices->push_back(irr::video::S3DVertex(x, y, z, 0, 1, 0, irr::video::SColor(255, 0, 255, 255), tu, tv));
			if (children.size() == 5) {
				//step down the vertex list
				children.at(5)->cumulateVertices(vertices);
			}
		}
	}

	void readSceneCommands(irr::scene::ISceneManager* smgr, std::vector<loadedmesh>* mastermeshes) {
		Tree* sceneAction = this;
		if (sceneAction->children.at(0)->tag.compare("MESHADD") == 0) {
			std::string meshname = sceneAction->children.at(0)->children.at(0)->lexeme;
			for (int i = 0; i < mastermeshes->size(); i++) { //find mesh with name
				if (mastermeshes->at(i).name.compare(meshname) == 0) {
					IMeshSceneNode* myNode = smgr->addMeshSceneNode(mastermeshes->at(i).mesh); //add mesh with name to the scene

					myNode->setName((meshname + "_" + std::to_string(smgr->getRootSceneNode()->getChildren().size())).c_str());
					myNode->setID(smgr->getRootSceneNode()->getChildren().size());

					myNode->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
					myNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
					myNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, false);
				}
			}
		}
		if (sceneAction->children.size() == 2) {
			children.at(1)->readSceneCommands(smgr, mastermeshes);
		}
	}

	void generateScene(irr::scene::ISceneManager* smgr, std::vector<loadedmesh>* meshes) { //Children are a 1. descriptor or 2. next descriptor node
		for (auto child : children) {
			if (child->tag.compare("MESH") == 0) {
				std::string name = children.at(0)->children.at(0)->lexeme;
				std::vector<irr::video::S3DVertex> vertices;
				child->children.at(1)->cumulateVertices(&vertices);
				std::cout << "Vert size: " << vertices.size() << "\n";

				SMesh* mesh = new SMesh();

				SMeshBuffer *buf = 0;
				buf = new SMeshBuffer();
				mesh->addMeshBuffer(buf);
				buf->drop();

				buf->Vertices.reallocate(vertices.size());
				buf->Vertices.set_used(vertices.size());
				buf->Indices.reallocate(vertices.size());
				buf->Indices.set_used(vertices.size());

				for (int i = 0; i < vertices.size(); i++) {
					buf->Vertices[i] = vertices[i];
					buf->Indices[i] = i;
				}

				buf->Indices.reallocate(vertices.size());
				buf->Indices.set_used(vertices.size());

				buf->recalculateBoundingBox();

				meshes->push_back(loadedmesh(mesh, name));
			}
			else if (child->tag.compare("SCENE") == 0) {
				child->children.at(0)->readSceneCommands(smgr, meshes);

			}
			else if (child->tag.compare("DESCRIPTOR") == 0) {
				child->generateScene(smgr, meshes);
			}
		}
	}
};

/*

SMesh* mesh = new SMesh();

SMeshBuffer *buf = 0;
buf = new SMeshBuffer();
mesh->addMeshBuffer(buf);
buf->drop();

buf->Vertices.reallocate(subt-2);
buf->Vertices.set_used(subt- 2);
buf->Indices.reallocate(subt - 2);
buf->Indices.set_used(subt - 2);

for (int i = 0; i < vertices.size(); i++) {
std::cout << "Table[" << i << "] coordinates: " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << "\n";
buf->Vertices[i] = irr::video::S3DVertex(vertices[i][0], vertices[i][1], vertices[i][2], 0, 1, 0, irr::video::SColor(255, 0, 255, 255), 0, 1);
buf->Indices[i] = i;
}

buf->Indices.reallocate(subt-2);
buf->Indices.set_used(subt-2);

buf->recalculateBoundingBox();


IMeshSceneNode* myNode = device->getSceneManager()->addMeshSceneNode(mesh);


std::string n_name = makeName();
myNode->setName(n_name.c_str());
myNode->setID(smgr->getRootSceneNode()->getChildren().size());

myNode->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
myNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
myNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, false);

*/