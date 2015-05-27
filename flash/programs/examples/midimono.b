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
				"type" : "USBMonophoncMidiController",
				"gate" : 2,
				"pitch" : 1,
				"velocity" : 3,
				"pressure" : 5,
				"aftertouch" : 4,
				"triggerOn" : 3,
				"triggerOff" : 4
			}
		]
	}
}