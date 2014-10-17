{
	"program" : 	

	{
		"name" : 			"Probability Drum Trigger Sequence Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"name" : "snare",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 3,
				"probabilityModifier" : 3,
				"velocityRange" : 250,
				"triggers" :   [ 0, 0, 0, 5, 		75, 0, 0, 0, 	0, 0, 0, 8, 	75, 0, 0, 1000 ],
				"velocities" : [ 0, 0, 0, 1000, 	5000, 0, 0, 0, 	0, 0, 0, 1500, 	5000, 0, 0, 1000 ]
			},

			{
				"name" : "kick",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 3,
				"probabilityModifier" : 3,
				"velocityRange" : 250,
				"triggers" :   [ 75, 0, 0, 0, 		0, 0, 0, 10, 	75, 0, 0, 0, 	0, 0, 0, 25 ],
				"velocities" : [ 5000, 0, 0, 0, 	0, 0, 0, 1000, 	5000, 0, 0, 0, 	0, 0, 0, 1000 ]
			}					
		]
	}
}