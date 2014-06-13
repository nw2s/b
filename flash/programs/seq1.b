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
				"analogOutput" : 2,
				"gateOutput" : 1,
				"gateLength" : 125,
				"root" : "C",
				"scale" : "major",
				"division" : "sixteenth",
				"randomize" : true,
				"notes" : [
					
					[1,1], [0,0], [1,5], [1,4],
					[1,1], [1,4], [0,0], [1,5],
					[1,1], [1,5], [1,3], [0,0],
					[1,1], [1,5], [1,4], [1,3]
					
				]
			}		
		]
	}
}