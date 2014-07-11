{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
						
					
			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 1,
				"probabilityModifier" : 1,
				"triggers" :   [ 10 ]
			},

			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 1,
				"probabilityModifier" : 2,
				"triggers" :   [ 10 ]
			},

			{
				"type" : "Looper",
				"subfolder" : "melodic",
				"filename" : "ebgtr120.raw",
				"samplerate" : "24000",
				"glitch" : 1,
				"dacOutput" : 1
			},

			{
				"type" : "Looper",
				"subfolder" : "drum",
				"filename" : "drum01.raw",
				"samplerate" : "24000",
				"glitch" : 2,
				"dacOutput" : 2
			}
							
		]
	}
}

