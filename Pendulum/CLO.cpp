#include "CLO.hpp"

void except_caught(const cl::Error& err, const std::string& hol)
{
	std::cerr
		 << "In CLO::" << hol << "\n"
         << "ERROR: "
         << err.what()
         << "("<< err.err()<< ")"
		 << "\n" << error_map[err.err()] 
         << std::endl;
		throw std::exception();
		//Igen, elég fura gondolat kidobni egy errort, ha elkaptunk egyet
		//De most kb. wrappelem a wrappelt cl-es dolgokat, és nem akarok kiengedni
		//cl::Error-okat, ha itt valami baj van, akkor azt kiírom, és dob egy exceptiont
}

void CLO::setPlatform(bool autoselect)
{
	try
	{
		//Lekérjük a platformokat
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0)//Ha nincs, akkor
		{
			//egyrészt közöljük
			std::cerr << "Platform size 0\n";
			//másrészt egy kicsit noszogatjuk magunkat arra, hogy ne hagyjuk továbbfajulni a dolgokat
			throw std::runtime_error("No platforms found!");//csak cl::Error-okat catchelünk itt, ez átmegy
		}
		
		if(autoselect)	
			platf = platforms[0];
		else
		{
			std::cerr << "Found "<< platforms.size() << " platforms:" <<"\n";
			//Kilistázzuk, index-el együtt
			for(unsigned short i = 0; i < platforms.size(); i++)
			{
				std::string str;
				platforms[i].getInfo(CL_PLATFORM_NAME, &str);
				std::cerr << i << " - " << str << std::endl;
				
				//std::cout<<x.name()<<std::endl;
			}
			
			std::cerr << "Select a platform:" << std::endl;
			
			//És bekérünk egy létező indexet
			unsigned int ind = 0;
			while(!(std::cin >> ind && ind >= 0 && ind < platforms.size()))
			{
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cerr << "Wrong id. Try again:\n";
			}
			//És kész is vagyunk, csak eltároljuk a választást
			platf = platforms[ind];
		}	
		
	}
	catch(const cl::Error& err)//Ha máris meghaltunk (vajon egyáltalán lehet ilyet?)
	{
		except_caught(err, "setPlatform");
	}
}

//TODO
/*
void CLO::setDevice(bool autoselect)
{
	
}*/

const cl::Platform& CLO::getPlatform() const
{
	return platf;
}

CLO::CLO(bool autoselect)
{
	//Platformválasztás
	setPlatform(autoselect);	
	
	try
	{
		//Eszközök betöltése
		platf.getDevices( CL_DEVICE_TYPE_ALL , &devices );		
		//Context létrehozása
		cont = cl::Context(devices);
		//CommandQueue-k létrehozása
		for(auto& dev : devices)
			queues.push_back(cl::CommandQueue(cont, dev));
	}
	catch(const cl::Error& err)
	{
		except_caught(err, "CLO");
	}
}

CLO::~CLO()
{
	
}

void CLO::loadProgram(const std::string& source_code)
{
	prog = cl::Program(cont, source_code);
	try {
		prog.build(devices);
	}
	catch (const cl::Error& err )
	{
		if (err.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			std::string str(100, ' ');
			std::cout << prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;//TODO belepiszkáltam
		}
		else
			throw err;
	}
}

void CLO::loadFile(const std::string& filename)//exceptiont dobál a filestream is
{
	std::ifstream t(filename);
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	t.close();
	loadProgram(str);
}

std::vector<cl::Kernel> CLO::getKernels(const std::vector<std::string>& names) const
{
	std::vector<cl::Kernel> ret;
	try
	{
		for(auto& name : names)
			ret.push_back(cl::Kernel(prog, name.c_str()));
	}
	catch(const cl::Error& err)
	{
		except_caught(err, "getKernels");
	}
	
	return ret;
}



unsigned int CLO::getDeviceNumber() const
{
	return devices.size();
}

const std::vector<cl::Device>& CLO::getDevices() const
{
	return devices;
}

std::vector<std::pair<cl::Device, cl::CommandQueue> > CLO::getStuff() const
{
	std::vector<std::pair<cl::Device, cl::CommandQueue> > ret;
	for(unsigned int i = 0; i < devices.size(); i++)
		ret.push_back(std::pair<cl::Device, cl::CommandQueue>(devices[i], queues[i]));
	return ret;
}

CLO CLO::fromFile(std::string filename, bool autoselect)
{
	CLO ret(autoselect);
	ret.loadFile(filename);
	return ret;
}
		
CLO CLO::fromString(std::string str, bool autoselect)
{
	CLO ret(autoselect);
	ret.loadFile(str);
	return ret;
}

/*
#include <cstdio>
#include <cstdlib>
#include <iostream>

const char * helloStr  = "__kernel void "
                         "hello(void) "
                         "{ "
                         "  "
                         "} ";*/

/*
						 
int main(void)
{
   

     cl_context_properties properties[] = 
        { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
     cl::Context context(CL_DEVICE_TYPE_CPU, properties); 

     std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     cl::Program::Sources source(1,
         std::make_pair(helloStr,strlen(helloStr)));
     cl::Program program_ = cl::Program(context, source);
     program_.build(devices);

     cl::Kernel kernel(program_, "hello", &err);

     cl::Event event;
     cl::CommandQueue queue(context, devices[0], 0, &err);
     queue.enqueueNDRangeKernel(
         kernel, 
         cl::NullRange, 
         cl::NDRange(4,4),
         cl::NullRange,
         NULL,
         &event); 

     event.wait();
   }
   catch (cl::Error err) {
      std::cerr 
         << "ERROR: "
         << err.what()
         << "("
         << err.err()
         << ")"
         << std::endl;
   }

  return EXIT_SUCCESS;
}
*/