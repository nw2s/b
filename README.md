b
=

nw2s::b is a generalized framework and set of tools for controlling modular synthesizers using microcontrollers such as the Arduino. It is designed to leverage the hardware controller of the same name, but will also run on the Ardcore and its expansion module. 

The goal of the framework is to provide a number of modular building blocks on which you can design custom sequenced and algorithmic compositional tools.

To that end, I've followed a few design principles: 

* **Flexibility of underlying hardware** - Standalone Arduinos are great for debugging. The Ardcore is a perfect development platform, but the nw2s::b platform unleashes the full potential of the microcontroller-based modular synth control. Why shouldn't it work on all of them?
* **Ease of implementation** - Just because it's C++ doesn't mean it has to be hard. Building a controller out of the pre-build devices should be easy and straightforward.
* **Separation of concerns** - Each piece of the framework should have a well-defined job and do it well. 

To demonstrate the implementation of these principles, the following code shows how to set up a basic sketch that will create a clock that is slaved to the external clock port and creates a few divided trigger signals on the digital outputs. The code should be fairly easy to follow:

	using namespace nw2s;
	    
	void setup() 
	{
		EventManager::initialize();
		/* Clock slaving to the Ardcore clock input */
		Clock* democlock = SlaveClock::create(ARDCORE_CLOCK_IN, 16);
	
		/* Register the clock with the EventManager */
		EventManager::registerdevice(democlock);
	
		Trigger* trigger0 = Trigger::create(ARDCORE_OUT_EX_D0, DIV_QUARTER);
		democlock->registerdevice(trigger0);
	
		Trigger* trigger5 = Trigger::create(ARDCORE_OUT_EX_D5, DIV_WHOLE);
		democlock->registerdevice(trigger5);
	
		Trigger* trigger6 = Trigger::create(ARDCORE_OUT_EX_D6, DIV_EIGHTH_TRIPLET);
		democlock->registerdevice(trigger6);
	
	}

	void loop() 
	{
		EventManager::loop();	
	}

To run this code on the nw2s::b hardware, which uses a completely different microprocessor architecture, you simply change the port names to match your configuration, compile, upload, and run. For more specific examples, skip to [Tutorial](https://github.com/nw2s/b/wiki/03-Tutorial:-Clocks). For more details on how to get up and running, continue to [Setup](https://github.com/nw2s/b/wiki/02-Setup)