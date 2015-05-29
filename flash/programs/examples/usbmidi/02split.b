{
	"program" : 	

	{
		"name" : 			"Split Monophonic Midi",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "USBSplitMonoMidiController",
				
				"gate1" : 1,
				"pitch1" : 1,
				"velocity1" : 2,
				"pressure1" : 3,
				"triggerOn1" : 2,
				"triggerOff1" : 3,
				
				"gate2" : 4,
				"pitch2" : 4,
				"velocity2" : 5,
				"pressure2" : 6,
				"triggerOn2" : 5,
				"triggerOff2" : 6,
				
				"aftertouch" : 7,
				"splitNote" : 48,
				
				"controllerMap" : [
					
					{
						"controlNumber" : 1,
						"output" : 7,
						"bipolar" : false
					},
					{
						"controlNumber" : 103,
						"output" : 8,
						"bipolar" : false
					},
					{
						"controlNumber" : 102,
						"output" : 9,
						"bipolar" : false
					},
					{
						"controlNumber" : 14,
						"output" : 10,
						"bipolar" : false
					},
					{
						"controlNumber" : 15,
						"output" : 11,
						"bipolar" : false
					},
					{
						"controlNumber" : 16,
						"output" : 12,
						"bipolar" : false
					},
					{
						"controlNumber" : 17,
						"output" : 13,
						"bipolar" : false
					},
					{
						"controlNumber" : 18,
						"output" : 14,
						"bipolar" : false
					},
					{
						"controlNumber" : 19,
						"output" : 15,
						"bipolar" : false
					},
					{
						"controlNumber" : 20,
						"output" : 16,
						"bipolar" : false
					}
				]
			}
		]
	}
}