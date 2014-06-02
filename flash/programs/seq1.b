{
	"program" : 	

	{
		"name" : 			"Sequence Demo 1",

		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},
			
		"devices" : [
			
			{
				"type" : "NoteSequencer",
				"analogOutput" : 1,
				"root" : "C",
				"scale" : "MAJOR",
				"division" : "EIGHTH",
				"notes" : [
					
					[1,1], [1,3], [1,5], [2,1]
					
				],
			}		
		]
	}
}