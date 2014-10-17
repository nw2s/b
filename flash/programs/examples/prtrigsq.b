


{
	"program" : 	

	{
		"name" : 			"Probability Trigger Sequence Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"name" : "snare",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 1,
				"probabilityModifier" : 1,
				"triggers" :   [ 0, 5, 0, 5, 80, 0, 0, 15, 0, 0, 0, 5, 80, 5, 5, 0 ]
			},

			{
				"name" : "open hat",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 2,
				"probabilityModifier" : 2,
				"triggers" :   [ 0, 0, 0, 0, 0, 0, 5 ]
			},

			{
				"name" : "hat",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 3,
				"probabilityModifier" : 3,
				"triggers" :   [ 5, 50, 0, 50, 5, 50, 0, 50, 5, 50, 0, 50, 5, 50, 0 ]
			},

			{
				"name" : "kick",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 4,
				"probabilityModifier" : 4,
				"triggers" :   [ 90, 0, 0, 5, 0, 0, 5, 5, 90, 0, 0, 5, 0, 5, 0, 5 ]
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter triplet",
				"analogOutput" : 15,
				"min" : 0,
				"max" : 5000
			},		
					
			{
				"type" : "CVSequencer",
				"division" : "half",
				"analogOutput" : 16,
				"min" : 0,
				"max" : 5000
			}		
					
		]
	}
}



