# Interface StepListManagerInterface

Interface que contém os métodos chamados pelos itens da lista de passos de uma história para efetivar alterações em um item da lista. Contém as funções:

1. OnStepChanged: para mudança em qualquer característica do passo;
2. OnStepDeleted: para remoção do passo;
3. GetStepsClassesMapKeys: para receber as strings correspondentes às possíveis classes de passo, utilizadas no dropdown da entrada;
4. OpenMessageEditorForStep: para abrir o editor de mensagens de erro do passo.