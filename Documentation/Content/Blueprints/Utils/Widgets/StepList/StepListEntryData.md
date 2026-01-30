# Struct StepListEntryData

Contém o struct com os dados necessários para a exibição dos elementos da lista de passos de uma história:

* StoryIndex: index da história que contém o passo, utilizado em conjunto com o StepIndex para identificar o passo e realizar operações de Get e Set;
* StepIndex: index do passo, utilizado para operações de Get e Set;
* SelectedStepClass: String correspondente ao nome da classe do passo, mapeada para o tipo da classe na instanciação do passo;
* SelectedStepAnchor: tag da âncora associada ao passo.