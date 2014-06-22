{
	"program" : 	

	{
		"name" : 			"Drum Trigger Sequence Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "DrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 3,
				"triggers" : [
					
					1000, 0, 0, 0, 5000, 0, 2500, 0, 1000, 0, 0, 2500, 0, 1000, 0, 1000
															
				]
			},

			{
				"type" : "DrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 4,
				"triggers" : [
					
					1000, 0, 0, 0, 5000, 0, 2500, 0, 1000, 0, 0, 2500, 0, 1000, 0
															
				]
			}
					
		]
	}
}