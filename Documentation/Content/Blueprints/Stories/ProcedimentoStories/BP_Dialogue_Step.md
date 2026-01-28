# Blueprint BP_Dialogue_Step

Blueprint que herda da classe BP_StepBase e coordena a inicialização do Widget de Diálogo. 

## EventGraph

*** Considerando apenas a existência de um widget de diálogo uma única vez na sessão ***
*** Trocar para busca de tag para poder ser facilmente utilzado ***

Após identificar o widget de diálogo na cena, o blueprint torna o widget visível, associa os dispatchers do widget ao tratamento correto e inicia a exibição dos diálogos.

O dispatcher OnWrongAnswerSelected é associado ao evento WrongAnswer que inicia a rotina TreatWrongAnswer, no futuro, é possível acoplar nesta seção uma chamada para uma função que computabilize os erros do usuário.

O dispatcher OnCorrectAnswerSelected é associado ao evento CorrectAnswer, que não é utilizado no momento, porém poderá ser acoplado a uma rotina que computabilize os acertos do usuário. 

Finalmente, o dispatcher OnAnswersEnded é associado ao evento AnswersEnded que chama o dispatcher OnStepFinished. 
