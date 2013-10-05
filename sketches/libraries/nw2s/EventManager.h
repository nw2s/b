
#ifndef EventManager_h
#define EventManager_h

#include <iterator>
#include <vector>

using namespace std;

namespace nw2s
{		
	class EventManager;
	class TimeBasedDevice;
}

class nw2s::TimeBasedDevice
{
	public:
		virtual void timer(unsigned long t) = 0;

};

class nw2s::EventManager
{
	public:
		static void initialize();
 		static void registerdevice(TimeBasedDevice* device);
		static void loop();
	
	private:
		static volatile unsigned long t;
		static vector<TimeBasedDevice*> timedevices;		
};


#endif


