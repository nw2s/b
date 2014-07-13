
{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 2",
			
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
			}
							
		]
	}
}

