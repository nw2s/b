{
	"program" : 	

	{
		"name" : 			"Default Demo",

		"clock" :
		
		{
	        "type" : "VariableClock",
	        "minTempo" : 30,
	        "maxTempo" : 240,
	        "tempoInput" : 1,
	        "beats" : 16            
		},
		

		"devices" : [			
			
			
			{
				"type" : "CVNoteSequencer",
				"analogOutput" : 1,
				"root" : "C",
				"scale" : "major pentatonic",
				"randomize" : true,
				"notes" : [
					
					[1,1], [1,2], [1,3], [1,4], [1,5],
					[2,1], [2,2], [2,3], [2,4], [2,5],
					[3,1], [3,2], [3,3], [3,4], [3,5]
					
				]
			},
					
			{
				"type" : "MorphingNoteSequencer",
				"analogOutput" : 2,
				"chaos" : 5,
				"root" : "C",
				"scale" : "major",
				"division" : "sixteenth",
				"reset" : 1,
				"notes" : [
					
					[1,1], [0,0], [1,2], [1,3],
					[1,4], [1,5], [0,0], [1,6],
					[1,7], [2,1], [2,2], [0,0],
					[2,3], [2,4], [2,5], [2,6],
					[2,7], [3,1]
					
				]
			},		
			
			{
				"name" : "snare",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 3,
				"probabilityModifier" : 7,
				"velocityRange" : 250,
				"triggers" :   [ 0, 0, 0, 5, 		75, 0, 0, 0, 	0, 0, 0, 8, 	75, 0, 0, 1000 ],
				"velocities" : [ 0, 0, 0, 1000, 	5000, 0, 0, 0, 	0, 0, 0, 1500, 	5000, 0, 0, 1000 ]
			},

			{
				"name" : "hat 1",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 6,
				"probabilityModifier" : 7,
				"velocityRange" : 250,
				"triggers" :   [ 0, 25, 5, 25, 			0, 25, 5, 25, 			0, 25, 5, 25 ],
				"velocities" : [ 0, 1000, 100, 1000, 	0, 1000, 2000, 1000, 	0, 1000, 750, 1500 ]
			},

			{
				"name" : "hat 2",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "quarter",
				"analogOutput" : 5,
				"probabilityModifier" : 7,
				"velocityRange" : 250,
				"triggers" :   [ 0, 5, 5, 25 ],
				"velocities" : [ 0, 1000, 100, 1000 ]
			},

			{
				"name" : "kick",
				"type" : "ProbabilityDrumTriggerSequencer",
				"division" : "sixteenth",
				"analogOutput" : 4,
				"probabilityModifier" : 8,
				"velocityRange" : 250,
				"triggers" :   [ 75, 0, 0, 0, 		0, 0, 0, 10, 	75, 0, 0, 0, 	0, 0, 0, 25 ],
				"velocities" : [ 5000, 0, 0, 0, 	0, 0, 0, 1000, 	5000, 0, 0, 0, 	0, 0, 0, 1000 ]
			},					

			{
				"name" : "snare",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 1,
				"probabilityModifier" : 2,
				"triggers" :   [ 0, 5, 0, 5, 80, 0, 0, 15, 0, 0, 0, 5, 80, 5, 5, 0 ]
			},


			{
				"type" : "CVSequencer",
				"division" : "whole",
				"analogOutput" : 7,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "half",
				"analogOutput" : 8,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "dotted quarter",
				"analogOutput" : 9,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "eighth triplet",
				"analogOutput" : 10,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "eighth",
				"analogOutput" : 11,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "dotted eighth",
				"analogOutput" : 12,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "triplet eighth",
				"analogOutput" : 13,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "dotted sixteenth",
				"analogOutput" : 14,
				"min" : 0,
				"max" : 5000
			},		

			{
				"name" : "open hat",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 2,
				"probabilityModifier" : 3,
				"triggers" :   [ 0, 0, 0, 0, 0, 0, 5 ]
			},

			{
				"name" : "hat",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 3,
				"probabilityModifier" : 4,
				"triggers" :   [ 5, 50, 0, 50, 5, 50, 0, 50, 5, 50, 0, 50, 5, 50, 0 ]
			},

			{
				"name" : "kick",
				"type" : "ProbabilityTriggerSequencer",
				"division" : "sixteenth",
				"digitalOutput" : 4,
				"probabilityModifier" : 5,
				"triggers" :   [ 90, 0, 0, 5, 0, 0, 5, 5, 90, 0, 0, 5, 0, 5, 0, 5 ]
			},

			{
				"name" : "snare 15/16",
				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 5,
				"triggers" : [
					
					0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
															
				]
			},

			{
				"name" : "hihat 13/16",
				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 6,
				"triggers" : [
					
					1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1
															
				]
			},

			{
				"name" : "kick 9/16",
 				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 7,
				"triggers" : [
					
					1, 0, 0, 0, 0, 0, 0, 1, 0
															
				]
			},

			{
				"name" : "Hat 5/16",
 				"type" : "TriggerSequencer",
				"division" : "sixteenth",
				"triggerOutput" : 8,
				"triggers" : [
					
					1, 0, 1, 0, 1
															
				]
			},


			{
				"type" : "RandomLoopingShiftRegister",
				"size" : 32,
				"division" : "sixteenth",
				"controlInput" : 2,
				"analogOutput" : 15,
				"delayedOutput" : 16,
				"delay" : 12,
				"triggerOutput1" : 9,
				"triggerOutput2" : 10,
				"triggerOutput3" : 11,
				"triggerOutput4" : 12,
				"triggerOutput5" : 13,
				"triggerOutput6" : 14,
				"triggerOutput7" : 15,
				"triggerOutput8" : 16
			}
		]
	}
}