# Widget StepListEntryWidget

Widget de um elemento da lista de passos de uma história. Contém um dropdown para a classe do passo, outro para a tag de âncora associada, um botão para abrir o widget MessageEditor para alterar as mensagens de erro do passo e um botão para remoção do passo.

## Graph

### Variáveis

#### IsInitializing

Variável utilizada para indicar se alterações de valores nos dropdowns são referentes ao procedimento de inicialização do item, ao passar os valores obtidos com o objeto de entrada. Dessa forma, evita que essas alterações triggem as rotinas de tratamento para alterações feitas pelo desenvolvedor no editor widget StoryBuilder durante a inicialização.

### EventGraph

#### OnListItemObjectSet

- Esse evento é chamado na criação do item da lista, através da passagem de um objeto com o formato StepListEntryObject dentro do editor widget StoryBuilder. 
    
Configura a variável StepListEntry do widget para referenciar o objeto de entrada, chama as duas funções para população dos dropdowns e atualiza o valor da variável IsInitializing para *false*.

#### OnSelectionChanged (StepAnchorTag)

Caso a alteração não corresponda à inicialização do item, chama o método OnStepAnchorChanged para tratar a mudança de tag de âncora associada ao passo.

#### OnSelectionChanged (StepClass)

Caso a alteração não corresponda à inicialização do item, chama o método OnStepClassChanged para tratar a mudança de classe do passo.

#### OnClicked (RemoveStepButton)

Após interagir com o botão de remoção do item, chama a rotina OnStepDeleted, responsável por acionar a interface StepListManagerInterface para remover o passo.

#### OnClicked (MessageEditorButton)

Após interagir com o botão do MessageEditor, aciona a interface StepListManagerInterface e chama o método OpenMessageEditorForStep para abrir o widget e exibir os recursos de edição das mensagens de erro do passo.

### Funções

#### FillStepClassesDropdown

Rotina que popula o dropdown de classes do passo.

Inicia com a limpeza dos itens do dropdown, aciona a interface StepListManagerInterface e chama o método GetStepsClassesMapKeys, obtendo a lista de classes disponíveis para os passos. Então, varre a lista, adicionando um item no dropdown para cada elemento. Finalmente, verifica se o valor recebido de classe do objeto de entrada existe na lista, se for o caso, deixa esse valor como o item selecionado. Do contrário, deixa a primeira entrada que corresponde à classe base BP_StepBase.

#### FillAnchorNamesDropdown

Rotina que popula o dropdown de tags de âncoras registradas.

Inicia com a limpeza dos itens do dropdown, aciona a interface StepListManagerInterface e chama o método GetAnchorsNamesMapKeys, obtendo a lista de tags de âncoras registradas . Então, varre a lista, adicionando um item no dropdown para cada elemento. Finalmente, verifica se o valor recebido de tag do objeto de entrada existe na lista, se for o caso, deixa esse valor como o item selecionado. Do contrário, deixa a primeira entrada que corresponde a uma mensagem de nenhuma tag selecionada.

#### OnStepAnchorChanged

Constrói um novo objeto da classe StepListEntryObject contendo o novo valor de tag de âncora e substitui a referência da variável StepListEntry. Por fim, chama o método StepChanged para acionar a interface StepListManagerInterface e salvar as alterações.

#### OnStepClassChanged

Constrói um novo objeto da classe StepListEntryObject contendo o novo valor de classe e substitui a referência da variável StepListEntry. Por fim, chama o método StepChanged para acionar a interface StepListManagerInterface e salvar as alterações.

#### OnStepDeleted

Aciona a interface StepListManagerInterface e chama o método OnStepDeleted e passa os indexes obtidos com o objeto de entrada.

#### StepChanged

Aciona a interface StepListManagerInterface e chama o método OnStepChanged e passa todas as propriedades do valor atual da variável StepListEntry.