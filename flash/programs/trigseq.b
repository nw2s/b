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
				"type" : "TriggerSequencer",
				"division" : "eighth",
				"triggerOutput" : 1,
				"triggers" : [
					
					1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1
															
				]
			},

			{
				"type" : "TriggerSequencer",
				"division" : "eighth",
				"triggerOutput" : 2,
				"triggers" : [
					
					1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0
															
				]
			}
					
		]
	}
}