#pragma once

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <limits>
#include <iostream>
#include <string>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <streambuf>

#include <map>
//Forward deklarálás, error_map.cpp, csak ezért nem csinálok .hpp-t,
//de nem akartam bedobni main.cpp-be, mert sok sor
extern std::map<int, std::string> error_map;

/*
//Egy CommandQueue Context nélkül
//Mert minket az érdekel, hogy a feladat lefusson,
//és azután az, hogy hogyan optimális elosztani
//ha így állunk neki, akkor jobb, hogyha a feladat
//nem csak 1 adott deviceon fut el, hanem
//bedobálom ide, hogy miket akarok és utána fordítom
//arra az eszközre amelyikre jobbnak látom


struct CommandList
{
	
	
};*/



class CLO
{
	protected:
		//A platform
		cl::Platform platf;
		
		//Context
		cl::Context cont;
		
		//A program
		cl::Program prog;
		
		//Eszközök
		std::vector<cl::Device> devices;
		
		//CommandQueue-k, minden device-nak 1
		std::vector<cl::CommandQueue> queues;
		
		//Kiválasztja a platformot
		void setPlatform(bool autoselect = true);
		
		//TODO
		/*//Kiválasztja az eszközöket
		void setDevices(bool autoselect = true);*/
		
	public:
		//Konstans referenciát ad vissza a platformra
		const cl::Platform& getPlatform() const;
	
		//Betölti és lefordítja a programot egy string-ből
		void loadProgram(const std::string& source_code);
	
		//Betölti és lefordítja a programot egy file-ból
		void loadFile(const std::string& filename);
	
		//Visszatér a vectorban megadott nevekkel rendelkező kernelekkel
		std::vector<cl::Kernel> getKernels(const std::vector<std::string>& names) const;
	
		//Visszaadja a betöltött eszközök számát
		unsigned int getDeviceNumber() const;
		
		//Most komolyan ezt commentelnem kell?
		const std::vector<cl::Device>& getDevices() const;
	
		//Device-CommandQueue párokat dob vissza
		std::vector<std::pair<cl::Device, cl::CommandQueue> > getStuff() const;
	
		//CLO inicializálása, platform, device, context, stb.
		CLO(bool autoselect = true);
		
		//Takarítás
		~CLO();
		
		const cl::Context& getContext()
		{
			return cont;
		}
		
		static CLO fromFile(std::string filename, bool autoselect = true);
		
		static CLO fromString(std::string str, bool autoselect = true);
};