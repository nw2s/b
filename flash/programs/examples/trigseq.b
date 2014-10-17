{
	"program" : 	

	{
		"name" : 			"Trigger Sequence Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"name" : "snare",
				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 1,
				"triggers" : [
					
					0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
															
				]
			},

			{
				"name" : "hihat",
				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 2,
				"triggers" : [
					
					1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0
															
				]
			},

			{
				"name" : "kick",
 				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 3,
				"triggers" : [
					
					1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
															
				]
			},
			
			{
				"type" : "CVSequencer",
				"division" : "half",
				"analogOutput" : 2,
				"gateOutput" : 4,
				"gateLength" : 125,
				"min" : 0,
				"max" : 5000
			}		
		
		]
	}
}