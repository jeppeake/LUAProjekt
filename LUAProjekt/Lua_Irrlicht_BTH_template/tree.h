#pragma once
#include <string>
#include <list>
#include <iostream>

#include <irrlicht.h>
#include <vector>
#include <string>

#include <math.h> 

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
			vertices->push_back(irr::video::S3DVertex(x, y, z, 0, 1, 0, irr::video::SColor(255, 0, 0, 0), 0, 1));
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
			vertices->push_back(irr::video::S3DVertex(x, y, z, 0, 1, 0, irr::video::SColor(255, 255, 255, 255), tu, tv));
			if (children.size() == 6) {
				//step down the vertex list
				children.at(5)->cumulateVertices(vertices);
			}
		}
	}

	void cumulateTexture(std::vector<irr::core::vector3df>* vectors) {
		float x = std::stof(children.at(0)->lexeme);
		float y = std::stof(children.at(1)->lexeme);
		float z = std::stof(children.at(2)->lexeme);
		vectors->push_back(irr::core::vector3df(x,y,z));
		if (children.size() == 4) {
			//step down the vertex list
			children.at(3)->cumulateTexture(vectors);
		}
	}

	void readSceneCommands(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver,  std::vector<loadedmesh>* mastermeshes) {
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
		else if (sceneAction->children.at(0)->tag.compare("BIND") == 0) {
			std::string texname = sceneAction->children.at(0)->children.at(0)->lexeme;
			if (sceneAction->children.at(0)->children.at(1)->tag.compare("MESHADD") == 0) {
				std::string meshname = sceneAction->children.at(0)->children.at(1)->children.at(0)->lexeme;
				for (int i = 0; i < mastermeshes->size(); i++) { //find mesh with name
					if (mastermeshes->at(i).name.compare(meshname) == 0) {
						IMeshSceneNode* myNode = smgr->addMeshSceneNode(mastermeshes->at(i).mesh); //add mesh with name to the scene

						std::string newname = (meshname + "_" + std::to_string(smgr->getRootSceneNode()->getChildren().size()));
						myNode->setName(newname.c_str());
						myNode->setID(smgr->getRootSceneNode()->getChildren().size());

						


						irr::io::path texture_p;
						texture_p.append(texname.c_str());

						myNode->setMaterialTexture(0, driver->getTexture(texture_p));
						myNode->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
						myNode->setMaterialFlag(irr::video::EMF_LIGHTING, false);
					}
				}
			}
		}
		if (sceneAction->children.size() == 2) {
			children.at(1)->readSceneCommands(smgr, driver, mastermeshes);
		}
	}

	void generateScene(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver, irr::IrrlichtDevice* device, std::vector<loadedmesh>* meshes) { //Children are a 1. descriptor or 2. next descriptor node
		for (auto child : children) {
			if (child->tag.compare("MESH") == 0) {
				std::string name = children.at(0)->children.at(0)->lexeme;
				std::vector<irr::video::S3DVertex> vertices;
				child->children.at(1)->cumulateVertices(&vertices);

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

				mesh->setMaterialFlag(EMF_BACK_FACE_CULLING, false);
				mesh->setMaterialFlag(EMF_LIGHTING, false);

				meshes->push_back(loadedmesh(mesh, name));
			}
			else if (child->tag.compare("TEXTURE") == 0) {
				std::string name = children.at(0)->children.at(0)->lexeme;
				std::vector<irr::core::vector3df> vectors;
				child->children.at(1)->cumulateTexture(&vectors);

				irr::video::SColor c;

				float sqr = sqrt(vectors.size());

				if (std::fmod(sqr, 2) != 0) {
					//error
					return;
				}

				int dim1 = (int)sqr;

				float** values = new float*[dim1 * dim1];
				irr::core::dimension2d<irr::u32> d2v(dim1, dim1);

				unsigned int* bf = new unsigned int[dim1 * dim1];
				unsigned int* bfc = new unsigned int[dim1 * dim1];

				for (int i = 0; i < vectors.size(); i++) {
					irr::video::SColor c;
					c.setAlpha(255);
					c.setRed(vectors[i].X*255);
					c.setBlue(vectors[i].Y*255);
					c.setGreen(vectors[i].Z*255);
					unsigned int* b = new unsigned int;
					c.getData(b, ECF_A8R8G8B8);
					//std::cout << *b << "\n";
					bf[i] = *b;
				}

				driver->convertColor(bf, ECF_A8R8G8B8, dim1 * dim1, bfc, driver->getColorFormat());

				irr::io::path p;
				p.append(name.c_str());

				irr::video::IImage* image = driver->createImageFromData(driver->getColorFormat(), d2v, bfc);
				device->getVideoDriver()->writeImageToFile(image, "texture.png");
				driver->addTexture(p, image);

			}
			else if (child->tag.compare("SCENE") == 0) {
				child->children.at(0)->readSceneCommands(smgr, driver, meshes);

			}
			else if (child->tag.compare("DESCRIPTOR") == 0) {
				child->generateScene(smgr, driver, device, meshes);
			}
		}
	}
};