{
	"program" : 	

	{
		"name" : 			"USB Monophonic Midi",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "USBSplitMonoMidiController",
				"gate1" : 2,
				"pitch1" : 1,
				"velocity1" : 3,
				"pressure1" : 4,
				"triggerOn1" : 3,
				"triggerOff1" : 4,
				"gate2" : 5,
				"pitch2" : 5,
				"velocity2" : 6,
				"pressure2" : 7,
				"triggerOn2" : 6,
				"triggerOff2" : 7,
				"aftertouch" : 10,
				"splitNote" : 48
			}
		]
	}
}