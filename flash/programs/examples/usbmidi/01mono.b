{
	"program" : 	

	{
		"name" : 			"Monophonic Midi",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "USBMonophoncMidiController",
				"gate" : 1,
				"pitch" : 1,
				"velocity" : 2,
				"pressure" : 3,
				"aftertouch" : 4,
				"triggerOn" : 2,
				"triggerOff" : 3,
				
				"controllerMap" : [
					
					{
						"controlNumber" : 1,
						"output" : 5,
						"bipolar" : false
					},
					{
						"controlNumber" : 103,
						"output" : 6,
						"bipolar" : false
					},
					{
						"controlNumber" : 102,
						"output" : 7,
						"bipolar" : false
					},
					{
						"controlNumber" : 14,
						"output" : 8,
						"bipolar" : false
					},
					{
						"controlNumber" : 15,
						"output" : 9,
						"bipolar" : false
					},
					{
						"controlNumber" : 16,
						"output" : 10,
						"bipolar" : false
					},
					{
						"controlNumber" : 17,
						"output" : 11,
						"bipolar" : false
					},
					{
						"controlNumber" : 18,
						"output" : 12,
						"bipolar" : false
					},
					{
						"controlNumber" : 19,
						"output" : 13,
						"bipolar" : false
					},
					{
						"controlNumber" : 20,
						"output" : 14,
						"bipolar" : false
					},
					{
						"controlNumber" : 21,
						"output" : 15,
						"bipolar" : false
					},
					{
						"controlNumber" : 22,
						"output" : 16,
						"bipolar" : false
					}
				]
				
			}
		]
	}
}