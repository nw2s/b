{
	"program" : 	

	{
		"name" : 			"Morphing Sequence Demo 1",

		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},
			
		"devices" : [
			
			{
				"type" : "MorphingNoteSequencer",
				"analogOutput" : 2,
				"gateOutput" : 1,
				"gateLength" : 125,
				"chaos" : 15,
				"root" : "C",
				"scale" : "major",
				"division" : "sixteenth",
				"notes" : [
					
					[1,1], [0,0], [1,2], [1,3],
					[1,4], [1,5], [0,0], [1,6],
					[1,7], [2,1], [2,2], [0,0],
					[2,3], [2,4], [2,5], [2,6],
					[2,7], [3,1]
					
				]
			}		
		]
	}
}