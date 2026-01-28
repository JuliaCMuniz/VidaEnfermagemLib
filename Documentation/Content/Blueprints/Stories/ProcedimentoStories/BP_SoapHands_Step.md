# Blueprint BP_SoapHands_Step

Blueprint que herda da classe BP_StepBase e associa o dispatcher OnAnyHandSoaped do objeto SoapDispenser com o dispatcher OnStepFinished.

Conforme verificado com a professora Simone, o procedimento de higienização de mãos se inicia com o enxague das mãos na torneira, seguido do uso do sabonete em pelo menos uma das mãos. Dessa forma, o trigger desse passo foi associado ao dispatcher OnAnyHandSoaped ao invés do trigger OnBothHandsSoaped.