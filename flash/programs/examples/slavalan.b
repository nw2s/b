{
	"program" : 	

	{
		"name" : 			"Alanesque",

		"clock" :
		
		{
	        "type" : "PassthruClock",
	        "tapInput" : 1,
	        "beats" : 16            
		},
		

		"devices" : [			
			
			{
				"type" : "RandomLoopingShiftRegister",
				"size" : 32,
				"division" : "sixteenth",
				"controlInput" : 2,
				"analogOutput" : 15,
				"delayedOutput" : 16,
				"delay" : 12,
				"triggerOutput1" : 5,
				"triggerOutput2" : 6,
				"triggerOutput3" : 7,
				"triggerOutput4" : 8,
				"triggerOutput5" : 9,
				"triggerOutput6" : 10,
				"triggerOutput7" : 11,
				"triggerOutput8" : 12,
				"logicalAND1" : 13,
				"logicalAND2" : 14,
				"logicalAND3" : 15,
				"logicalAND4" : 16
			}
		]
	}
}