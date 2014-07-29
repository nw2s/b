
{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 4",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 60,
			"beats" : 16			
		},

		"devices" : [
											
			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 1,
				"probabilityModifier" : 2,
				"triggers" :   [ 20 ]
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter",
				"analogOutput" : 15,
				"min" : 0,
				"max" : 5000
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter triplet",
				"analogOutput" : 16,
				"min" : 0,
				"max" : 5000
			},

			{
				"type" : "Looper",
				"loops" : [
						
					{
						"subfolder" : "vocal",
						"filename" : "chess.raw"						
					},
					
					{
						"subfolder" : "vocal",
						"filename" : "waste.raw"						
					}		
				], 
				"mixmode" : "blend",
				"mixinput" : 5,
				"samplerate" : "24000",
				"glitch" : 1,
				"reverse" : 2,
				"density" : 1,
				"dacOutput" : 1
			}
							
		]
	}
}

