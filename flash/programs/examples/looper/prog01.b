
{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 4",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 130,
			"beats" : 16			
		},

		"devices" : [
											
			{
				"type" : "Trigger",
				"division" : "whole",
				"triggerOutput" : 1
			},

			{
				"type" : "Trigger",
				"division" : "whole",
				"triggerOutput" : 3
			},

			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 2,
				"probabilityModifier" : 7,
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
						"subfolder" : "drum2",
						"filename" : "loop01.pcm"						
					},
					
					{
						"subfolder" : "drum2",
						"filename" : "loop02.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop03.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop10.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop11.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop20.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop21.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop30.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop31.pcm"						
					},		
					
					{
						"subfolder" : "drum2",
						"filename" : "loop32.pcm"						
					}
						
				], 
				
				"startcontrol" : 1,
				"lengthcontrol" : 2,
				"mixcontrol" : 3,
				"finelengthcontrol" : 4,
				"density" : 5,
				"bitcontrol" : 6,
				"mixmode" : "toggle",
				"samplerate" : "24000",
				"glitch" : 1,
				"reverse" : 2,
				"mixtrigger" : 3,
				"resettrigger" : 4,
				"dacOutput" : 1
			}
							
		]
	}
}

