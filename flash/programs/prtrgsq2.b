{
	"program" : 	

	{
		"name" : 			"Probability Trigger Sequence Demo 2",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"name" : "melody 1 trigger",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth triplet",
				"digitalOutput" : 14,
				"probabilityModifier" : 1,
				"triggers" :   [ 25, 0, 25, 25, 0, 25, 25, 0, 25, 25, 0, 25, 25, 0 ]
			},

			{
				"name" : "melody 1",
				"type" : "NoteSequencer",
				"division" : "sixteenth triplet",
				"analogOutput" : 14,
				"root" : "C",
				"scale" : "major pentatonic",
				"randomize" : true,
				"notes" : [
					
					[3,1], [3,2], [3,3], [3,4], [3,5], [0,0], [0,0], [0,0]
					
				]
			},
					
			{
				"name" : "melody 2 trigger",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "eighth",
				"digitalOutput" : 15,
				"probabilityModifier" : 2,
				"triggers" :   [ 0, 0, 0, 0, 0, 10, 50 ]
			},

			{
				"name" : "melody 2",
				"type" : "NoteSequencer",
				"division" : "whole",
				"analogOutput" : 15,
				"root" : "C",
				"scale" : "major pentatonic",
				"randomize" : true,
				"notes" : [
					
					[2,1], [2,2], [2,3], [2,4], [2,5], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]
					
				]
			},

			{
				"name" : "melody 3 trigger",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 16,
				"probabilityModifier" : 3,
				"triggers" :   [ 75, 10, 75, 10 ]
			},

			{
				"name" : "melody 3",
				"type" : "NoteSequencer",
				"division" : "whole",
				"analogOutput" : 16,
				"root" : "C",
				"scale" : "major pentatonic",
				"randomize" : true,
				"notes" : [
					
					[1,1], [1,3], [1,5]
					
				]
			},

			{
				"name" : "drone",
				"type" : "NoteSequencer",
				"division" : "whole",
				"analogOutput" : 10,
				"root" : "C",
				"scale" : "major pentatonic",
				"randomize" : true,
				"notes" : [
					
					[2,1]
					
				]
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter",
				"analogOutput" : 11,
				"min" : 0,
				"max" : 5000
			},		
					
			{
				"type" : "CVSequencer",
				"division" : "quarter triplet",
				"analogOutput" : 12,
				"min" : 0,
				"max" : 5000
			},		
					
			{
				"type" : "CVSequencer",
				"division" : "half",
				"analogOutput" : 13,
				"min" : 0,
				"max" : 5000
			}		
					
		]
	}
}



