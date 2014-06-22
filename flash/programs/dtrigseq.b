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
				"name" : "kick",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "eighth",
				"digitalOutput" : 3,
				"probabilityModifier" : 1,
				"triggers" :   [ 75, 0, 0, 5, 75, 0, 0, 10 ],
			},

			{
				"name" : "snare",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "eighth",
				"digitalOutput" : 4,
				"probabilityModifier" : 2,
				"triggers" :   [ 0,	5, 75, 0, 0, 10, 75, 0 ],
			}
					
		]
	}
}