# Struct AnswerListEntryData

Contém o struct com os dados necessários para a exibição dos elementos da lista de respostas da montagem do Quiz:

* DialogueIndex: index da questão que contém a resposta, utilizado em conjunto com o AnswerIndex para identificar a resposta e realizar operações de Get e Set;
* AnswerIndex: index da resposta, utilizado para operações de Get e Set;
* AnswerContent: texto da resposta;
* AnswerColor: cor do balão de resposta;
* IsCorrect: boolean que define se a resposta é do tipo correta ou não;
* Explanation: mensagem de erro exibida caso a resposta seja do tipo incorreta e seja selecionada.